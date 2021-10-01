/*
 * MIT License
 *
 * Copyright (c) 2021 micn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.The MIT License (MIT)
 */

#include "CacheLoader.h"
#include <QQmlIncubator>
#include <QTimer>

QMap<QUrl, QQuickItem*> CacheLoader::gCacheMap = QMap<QUrl, QQuickItem*>();
QQmlEngine* CacheLoader::gQmlEngine = nullptr;

void CacheLoader::setEngine(QQmlEngine* engine) {
    gQmlEngine = engine;
}

CacheLoader::CacheLoader(QQuickItem* parent) : QQuickItem(parent) {
    if (gQmlEngine == nullptr) {
        qWarning() << "qml engine object is nullptr. use CacheLoader::setEngine().";
    }
}

CacheLoader::~CacheLoader() {
    for (auto item : gCacheMap) {
        item->setParentItem(nullptr);
        item->setParent(nullptr);
        item->setEnabled(false);
        item->setVisible(false);
        item->deleteLater();
    }

    if (mComponent) {
        mComponent.reset();
    }

    mIncubator.clear();
}

void CacheLoader::cacheItem() {
    loadQML(true);
}

bool CacheLoader::active() const {
    return mActive;
}

void CacheLoader::setActive(bool newVal) {
    mActive = newVal;

    if (mActive == true && mItem == nullptr) {
        loadQML();
    } else if (mActive == false) {
        QQuickItem* quickItem = qobject_cast<QQuickItem*>(mItem);
        if (quickItem) {
            quickItem->setParentItem(nullptr);
        }
        mItem = nullptr;
    }
}

QUrl CacheLoader::source() const {
    return mUrl;
}

void CacheLoader::setSource(const QUrl& newVal) {
    mUrl = newVal;

    if (mActive && !mUrl.isEmpty()) {
        loadQML();
    } else if (mUrl.isEmpty()) {
        QQuickItem* quickItem = qobject_cast<QQuickItem*>(mItem);
        if (quickItem) {
            quickItem->setParentItem(nullptr);
        }
        mItem = nullptr;
    }
}

CacheLoader::Status CacheLoader::status() const {
    return static_cast<CacheLoader::Status>(mIncubator.status());
}

qreal CacheLoader::progress() const {
    if (mComponent == nullptr) {
        return 0.0;
    }

    return mComponent->progress();
}

bool CacheLoader::asynchronous() const {
    return mAsynchronous;
}

void CacheLoader::setAsynchronous(bool newVal) {
    mAsynchronous = newVal;
}

QObject* CacheLoader::item() const {
    return mItem;
}

void CacheLoader::loadQML(bool readOnly) {
    if (mUrl.isEmpty()) {
        qWarning() << "qml source is empty. use CacheLoader::setSource().";
        return;
    }

    auto cachedItem = gCacheMap.value(mUrl, nullptr);
    if (cachedItem && !readOnly) {
        QQuickItem* prevItem = qobject_cast<QQuickItem*>(mItem);
        cachedItem->setParentItem(this);
        if (prevItem) {
            prevItem->setParentItem(nullptr);
        }

        mItem = cachedItem;
        return;
    }

    if (gQmlEngine == nullptr) {
        qWarning() << "qml engine object is nullptr. use CacheLoader::setEngine().";
        return;
    }

    QQuickItem* createdItem = nullptr;
    mComponent = std::make_shared<QQmlComponent>(gQmlEngine, mUrl.toString());
    if (mAsynchronous) {
        mComponent->create(mIncubator);

        QTimer* timer = new QTimer;
        QObject::connect(timer, &QTimer::timeout, [this]() {
            qWarning() << "timeout. It took 1 second to read the qml file.";
            if (mIncubator.isLoading()) {
                mIncubator.forceCompletion();
            }

            mWaitForIncubator = false;
        });
        timer->setSingleShot(true);
        timer->start(1000);

        mWaitForIncubator = true;
        while (mWaitForIncubator) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 50);
            if (mIncubator.isReady()) {
                mWaitForIncubator = false;
            }
        }
        createdItem = qobject_cast<QQuickItem*>(mIncubator.object());

        timer->stop();
        timer->deleteLater();

    } else {
        createdItem = qobject_cast<QQuickItem*>(mComponent->create());
    }

    if (createdItem) {
        gCacheMap.insert(mUrl, createdItem);

        if (!readOnly) {
            QQuickItem* prevItem = qobject_cast<QQuickItem*>(mItem);
            createdItem->setParentItem(this);
            if (prevItem) {
                prevItem->setParentItem(nullptr);
            }

            mItem = createdItem;
        }
    } else {
        qWarning() << "failed to read the qml file.";
        qWarning() << mComponent->errorString();
    }

    mComponent.reset();
}
