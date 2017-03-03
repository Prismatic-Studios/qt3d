/****************************************************************************
**
** Copyright (C) 2017 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QTest>
#include <Qt3DAnimation/private/animationcliploader_p.h>
#include <Qt3DAnimation/private/animationutils_p.h>
#include <Qt3DAnimation/private/channelmapper_p.h>
#include <Qt3DAnimation/private/channelmapping_p.h>
#include <Qt3DAnimation/private/handler_p.h>
#include <Qt3DAnimation/private/managers_p.h>
#include <qbackendnodetester.h>
#include <testpostmanarbiter.h>

using namespace Qt3DAnimation::Animation;

Q_DECLARE_METATYPE(Qt3DAnimation::Animation::Handler*)
Q_DECLARE_METATYPE(QVector<ChannelMapping *>)
Q_DECLARE_METATYPE(ChannelMapper *)
Q_DECLARE_METATYPE(AnimationClipLoader *)
Q_DECLARE_METATYPE(QVector<MappingData>)

class tst_AnimationUtils : public Qt3DCore::QBackendNodeTester
{
    Q_OBJECT

public:
    ChannelMapping *createChannelMapping(Handler *handler,
                                         const QString &channelName,
                                         const Qt3DCore::QNodeId targetId,
                                         const QString &property,
                                         const char *propertyName,
                                         int type)
    {
        auto channelMappingId = Qt3DCore::QNodeId::createId();
        ChannelMapping *channelMapping = handler->channelMappingManager()->getOrCreateResource(channelMappingId);
        setPeerId(channelMapping, channelMappingId);
        channelMapping->setTargetId(targetId);
        channelMapping->setProperty(property);
        channelMapping->setPropertyName(propertyName);
        channelMapping->setChannelName(channelName);
        channelMapping->setType(type);
        return channelMapping;
    }

    ChannelMapper *createChannelMapper(Handler *handler,
                                       const QVector<Qt3DCore::QNodeId> &mappingIds)
    {
        auto channelMapperId = Qt3DCore::QNodeId::createId();
        ChannelMapper *channelMapper = handler->channelMapperManager()->getOrCreateResource(channelMapperId);
        setPeerId(channelMapper, channelMapperId);
        channelMapper->setMappingIds(mappingIds);
        return channelMapper;
    }

    AnimationClipLoader *createAnimationClipLoader(Handler *handler,
                                                   const QUrl &source)
    {
        auto clipId = Qt3DCore::QNodeId::createId();
        AnimationClipLoader *clip = handler->animationClipLoaderManager()->getOrCreateResource(clipId);
        setPeerId(clip, clipId);
        clip->setSource(source);
        clip->loadAnimation();
        return clip;
    }

private Q_SLOTS:
    void checkBuildPropertyMappings_data()
    {
        QTest::addColumn<Handler *>("handler");
        QTest::addColumn<QVector<ChannelMapping *>>("channelMappings");
        QTest::addColumn<ChannelMapper *>("channelMapper");
        QTest::addColumn<AnimationClipLoader *>("clip");
        QTest::addColumn<QVector<MappingData>>("expectedMappingData");

        auto handler = new Handler;
        auto channelMapping = createChannelMapping(handler,
                                                   QLatin1String("Location"),
                                                   Qt3DCore::QNodeId::createId(),
                                                   QLatin1String("translation"),
                                                   "translation",
                                                   static_cast<int>(QVariant::Vector3D));
        QVector<ChannelMapping *> channelMappings;
        channelMappings.push_back(channelMapping);

        // ... a channel mapper...
        auto channelMapper = createChannelMapper(handler, QVector<Qt3DCore::QNodeId>() << channelMapping->peerId());

        // ...and an animation clip
        auto clip = createAnimationClipLoader(handler, QUrl("qrc:/clip1.json"));

        QVector<MappingData> mappingData;
        MappingData mapping;
        mapping.targetId = channelMapping->targetId();
        mapping.propertyName = channelMapping->propertyName(); // Location
        mapping.type = channelMapping->type();
        mapping.channelIndices = QVector<int>() << 0 << 1 << 2; // Location X, Y, Z
        mappingData.push_back(mapping);

        QTest::newRow("clip1.json") << handler
                                    << channelMappings
                                    << channelMapper
                                    << clip
                                    << mappingData;
    }

    void checkBuildPropertyMappings()
    {
        // GIVEN
        QFETCH(Handler *, handler);
        QFETCH(QVector<ChannelMapping *>, channelMappings);
        QFETCH(ChannelMapper *, channelMapper);
        QFETCH(AnimationClipLoader *, clip);
        QFETCH(QVector<MappingData>, expectedMappingData);

        // WHEN
        // Build the mapping data for the above configuration
        QVector<MappingData> mappingData = buildPropertyMappings(handler, clip, channelMapper);

        // THEN
        QCOMPARE(mappingData.size(), expectedMappingData.size());
        for (int i = 0; i < mappingData.size(); ++i) {
            const auto mapping = mappingData[i];
            const auto expectedMapping = expectedMappingData[i];

            QCOMPARE(mapping.targetId, expectedMapping.targetId);
            QCOMPARE(mapping.propertyName, expectedMapping.propertyName);
            QCOMPARE(mapping.type, expectedMapping.type);
            QCOMPARE(mapping.channelIndices.size(), expectedMapping.channelIndices.size());
            for (int j = 0; j < mapping.channelIndices.size(); ++j) {
                QCOMPARE(mapping.channelIndices[j], expectedMapping.channelIndices[j]);
            }
        }

        // Cleanup
        delete handler;
    }

    void checkLocalTimeFromGlobalTime_data()
    {
        QTest::addColumn<double>("globalTime");
        QTest::addColumn<double>("globalStartTime");
        QTest::addColumn<double>("playbackRate");
        QTest::addColumn<double>("duration");
        QTest::addColumn<int>("loopCount");
        QTest::addColumn<double>("expectedLocalTime");
        QTest::addColumn<int>("expectedCurrentLoop");

        double globalTime;
        double globalStartTime;
        double playbackRate;
        double duration;
        int loopCount;
        double expectedLocalTime;
        int expectedCurrentLoop;

        globalTime = 0.0;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        expectedLocalTime = 0.0;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_global = 0")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 0.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        expectedLocalTime = 0.5;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_global = 0.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 1.0;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        expectedLocalTime = 1.0;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_global = 1.0")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = -0.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        expectedLocalTime = 0.0;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_global = -0.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 1.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 1;
        expectedLocalTime = 1.0;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, t_global = 1.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 0.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 2;
        expectedLocalTime = 0.5;
        expectedCurrentLoop = 0;
        QTest::newRow("simple, loopCount = 2, t_global = 0.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 1.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 2;
        expectedLocalTime = 0.5;
        expectedCurrentLoop = 1;
        QTest::newRow("simple, loopCount = 2, t_global = 1.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 3.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 2.0;
        loopCount = 2;
        expectedLocalTime = 1.5;
        expectedCurrentLoop = 1;
        QTest::newRow("duration = 2, loopCount = 2, t_global = 3.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 4.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 2.0;
        loopCount = 2;
        expectedLocalTime = 2.0;
        expectedCurrentLoop = 1;
        QTest::newRow("duration = 2, loopCount = 2, t_global = 4.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 1.5;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 0;
        expectedLocalTime = 0.5;
        expectedCurrentLoop = 1;
        QTest::newRow("simple, loopCount = inf, t_global = 1.5")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;

        globalTime = 10.2;
        globalStartTime = 0.0;
        playbackRate = 1.0;
        duration = 1.0;
        loopCount = 0;
        expectedLocalTime = 0.2;
        expectedCurrentLoop = 10;
        QTest::newRow("simple, loopCount = inf, t_global = 10.2")
                << globalTime << globalStartTime << playbackRate << duration << loopCount
                << expectedLocalTime << expectedCurrentLoop;
    }

    void checkLocalTimeFromGlobalTime()
    {
        // GIVEN
        QFETCH(double, globalTime);
        QFETCH(double, globalStartTime);
        QFETCH(double, playbackRate);
        QFETCH(double, duration);
        QFETCH(int, loopCount);
        QFETCH(double, expectedLocalTime);
        QFETCH(int, expectedCurrentLoop);

        // WHEN
        int actualCurrentLoop = 0;
        double actualLocalTime = localTimeFromGlobalTime(globalTime,
                                                         globalStartTime,
                                                         playbackRate,
                                                         duration,
                                                         loopCount,
                                                         actualCurrentLoop);

        // THEN
        QCOMPARE(actualCurrentLoop, expectedCurrentLoop);
        QCOMPARE(actualLocalTime, expectedLocalTime);
    }
};

QTEST_MAIN(tst_AnimationUtils)

#include "tst_animationutils.moc"
