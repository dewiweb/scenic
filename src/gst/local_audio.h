
// localVideo.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _LOCAL_AUDIO_H_
#define _LOCAL_AUDIO_H_

#include <tr1/memory>
#include "noncopyable.h"

class Pipeline;
class AudioSource;
class AudioSourceConfig;
class AudioLevel;

class LocalAudio : private boost::noncopyable
{
    public:
        LocalAudio(Pipeline &pipeline, const AudioSourceConfig &sourceConfig);

    private:
        std::tr1::shared_ptr<AudioSource> source_;
        std::tr1::shared_ptr<AudioLevel> level_;
};

#endif // _LOCAL_AUDIO_H_

