//-----------------------------------------------------------------------------
// Project     : ASIO SDK
//
// Category    : Interfaces
// Filename    : common/iasiodrv.h
// Created by  : Steinberg, 05/1996
// Description : IASIO - the ASIO v2 COM interface
//
//-----------------------------------------------------------------------------
// This file is part of a Steinberg SDK. It is subject to the license terms
// in the LICENSE file found in the top-level directory of this distribution
// and at www.steinberg.net/sdklicenses.
// No part of the SDK, including this file, may be copied, modified, propagated,
// or distributed except according to the terms contained in the LICENSE file.
//-----------------------------------------------------------------------------

#ifndef __IASIODRV_H
#define __IASIODRV_H

#include "asio.h"

// {8F56ADB6-3B9F-11D4-8B6C-006097FBE174}
DEFINE_GUID(IID_IASIO, 0x8F56ADB6, 0x3B9F, 0x11D4,
            0x8B, 0x6C, 0x00, 0x60, 0x97, 0xFB, 0xE1, 0x74);

DECLARE_INTERFACE_(IASIO, IUnknown)
{
    virtual ASIOBool   init(void *sysRef) = 0;
    virtual void       getDriverName(char *name) = 0;
    virtual long       getDriverVersion() = 0;
    virtual void       getErrorMessage(char *string) = 0;
    virtual ASIOError  start() = 0;
    virtual ASIOError  stop() = 0;
    virtual ASIOError  getChannels(long *numInputChannels, long *numOutputChannels) = 0;
    virtual ASIOError  getLatencies(long *inputLatency, long *outputLatency) = 0;
    virtual ASIOError  getBufferSize(long *minSize, long *maxSize,
                           long *preferredSize, long *granularity) = 0;
    virtual ASIOError  canSampleRate(ASIOSampleRate sampleRate) = 0;
    virtual ASIOError  getSampleRate(ASIOSampleRate *sampleRate) = 0;
    virtual ASIOError  setSampleRate(ASIOSampleRate sampleRate) = 0;
    virtual ASIOError  getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
    virtual ASIOError  setClockSource(long reference) = 0;
    virtual ASIOError  getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
    virtual ASIOError  getChannelInfo(ASIOChannelInfo *info) = 0;
    virtual ASIOError  createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
                           long bufferSize, ASIOCallbacks *callbacks) = 0;
    virtual ASIOError  disposeBuffers() = 0;
    virtual ASIOError  controlPanel() = 0;
    virtual ASIOError  future(long selector, void *opt) = 0;
    virtual ASIOError  outputReady() = 0;
};

#endif // __IASIODRV_H
