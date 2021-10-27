#include "CacheLoader.h"

#include <QCoreApplication>
#include <QQmlIncubator>
#include <QTimer>

QMap<QUrl, QQuickItem*> CacheLoader::gCacheMap = QMap<QUrl, QQuickItem*>();
QQmlEngine* CacheLoader::gQmlEngine = nullptr;

void CacheLoader::setEngine(QQmlEngine* engine) {
    gQmlEngine = engine;
}

CacheLoader::CacheLoader(QQuickItem* parent) : QQuickItem(parent) {
    if (gQmlEngine == nullptr) {
        qWarning() << "[CacheLoader] qml engine object is nullptr. use CacheLoader::setEngine().";
    }
}

CacheLoader::~CacheLoader() {
    QQuickItem* quickItem = qobject_cast<QQuickItem*>(mItem);
    if (quickItem) {
        quickItem->setParentItem(nullptr);
        quickItem->setParent(nullptr);
        quickItem->setEnabled(false);
    }

    mItem = nullptr;

    if (mIncubator.isLoading()) {
        mIncubator.forceCompletion();
        mIncubator.clear();
    } else {
        mIncubator.clear();
    }

    if (mComponent) {
        mComponent.reset();
    }
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
        QMetaObject::invokeMethod(this, "loadQML", Qt::QueuedConnection);
    } else if (mActive == false) {
        QQuickItem* quickItem = qobject_cast<QQuickItem*>(mItem);
        if (quickItem) {
            quickItem->setEnabled(false);
            quickItem->setParent(nullptr);
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
        QMetaObject::invokeMethod(this, "loadQML", Qt::QueuedConnection);
    } else if (mUrl.isEmpty()) {
        QQuickItem* quickItem = qobject_cast<QQuickItem*>(mItem);
        if (quickItem) {
            quickItem->setEnabled(false);
            quickItem->setParent(nullptr);
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
        qWarning() << "[CacheLoader] qml source is empty. use CacheLoader::setSource().";
        return;
    }

    if (gCacheMap.contains(mUrl)) {
        auto cachedItem = gCacheMap.value(mUrl, nullptr);
        if (cachedItem && !readOnly) {
            QQuickItem* prevItem = qobject_cast<QQuickItem*>(mItem);
            if (cachedItem == mItem) {
                if (prevItem) {
                    prevItem->setEnabled(true);
                    prevItem->setParent(this);
                    prevItem->setParentItem(this);
                }
            } else {
                cachedItem->setEnabled(true);
                cachedItem->setParent(this);
                cachedItem->setParentItem(this);
                if (prevItem) {
                    prevItem->setEnabled(false);
                    cachedItem->setParent(nullptr);
                    prevItem->setParentItem(nullptr);
                }

                mItem = cachedItem;
            }

            emit loaded();
            return;
        } else if (!cachedItem) {
            gCacheMap.remove(mUrl);
        }
    }

    if (gQmlEngine == nullptr) {
        qWarning() << "[CacheLoader] qml engine object is nullptr. use CacheLoader::setEngine().";
        return;
    }

    QQuickItem* createdItem = nullptr;
    mComponent = std::make_shared<QQmlComponent>(gQmlEngine, mUrl.toString());
    if (mAsynchronous) {
        mComponent->create(mIncubator);
        QTimer* timer = new QTimer;
        QObject::connect(timer, &QTimer::timeout, [this]() {
            qWarning() << "[CacheLoader] timeout. It took 1 second to read the qml file.";
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
            createdItem->setSize(QSizeF(this->width(), this->height()));
            createdItem->setParent(this);
            createdItem->setParentItem(this);
            createdItem->setEnabled(true);
            QQuickItem* prevItem = qobject_cast<QQuickItem*>(mItem);
            if (prevItem) {
                prevItem->setEnabled(false);
                prevItem->setParent(nullptr);
                prevItem->setParentItem(nullptr);
            }
            mItem = createdItem;
        }
    } else {
        qWarning() << "[CacheLoader] failed to read the qml file.";
        qWarning() << mComponent->errorString();
        mComponent.reset();
        return;
    }

    emit loaded();
    mComponent.reset();
}
