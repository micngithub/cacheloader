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
#pragma once

#include <QMap>
#include <QQmlComponent>
#include <QQmlIncubator>
#include <QQuickItem>

class CacheLoader : public QQuickItem {
    Q_OBJECT

    Q_PROPERTY(bool active READ active WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(QObject* item READ item NOTIFY itemChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(qreal progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(bool asynchronous READ asynchronous WRITE setAsynchronous NOTIFY asynchronousChanged)

public:
    enum Status { Null, Ready, Loading, Error };
    Q_ENUM(Status)

    static void setEngine(QQmlEngine* engine);

    explicit CacheLoader(QQuickItem* parent = nullptr);
    ~CacheLoader() override;

    void cacheItem();

    Q_INVOKABLE bool active() const;
    Q_INVOKABLE void setActive(bool newVal);

    Q_INVOKABLE QUrl source() const;
    Q_INVOKABLE void setSource(const QUrl&);

    Q_INVOKABLE Status status() const;
    Q_INVOKABLE qreal progress() const;

    Q_INVOKABLE bool asynchronous() const;
    Q_INVOKABLE void setAsynchronous(bool a);

    Q_INVOKABLE QObject* item() const;

Q_SIGNALS:
    void itemChanged();
    void activeChanged();
    void sourceChanged();
    void sourceComponentChanged();
    void statusChanged();
    void progressChanged();
    void loaded();
    void asynchronousChanged();

private:
    void loadQML(bool readOnly = false);

    bool mActive = false;
    QUrl mUrl = QUrl("");
    QObject* mItem = nullptr;
    bool mAsynchronous = false;
    QQmlIncubator mIncubator;
    std::shared_ptr<QQmlComponent> mComponent;
    bool mWaitForIncubator = false;

    static QMap<QUrl, QQuickItem*> gCacheMap;
    static QQmlEngine* gQmlEngine;
};
