/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * StreamDecoderMock.cpp
 * A mock stream decoder.
 * Copyright (C) 2015 Simon Newton
 */

#include "StreamDecoderMock.h"

#include <gmock/gmock.h>

namespace {
MockStreamDecoder *g_stream_decoder_mock = NULL;
}

void StreamDecoder_SetMock(MockStreamDecoder* mock) {
  g_stream_decoder_mock = mock;
}

void StreamDecoder_Process(const uint8_t* data, unsigned int size) {
  if (g_stream_decoder_mock) {
    g_stream_decoder_mock->Send(data, size);
  }
}
