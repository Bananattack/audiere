/* libFLAC++ - Free Lossless Audio Codec library
 * Copyright (C) 2002  Josh Coalson
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "FLAC++/decoder.h"
#include "FLAC/assert.h"

namespace FLAC {
	namespace Decoder {

		const char *File::State::resolved_as_cstring(const File &decoder) const
		{
			if(state_ == ::FLAC__FILE_DECODER_SEEKABLE_STREAM_DECODER_ERROR) {
				FLAC::Decoder::SeekableStream::State state__ = decoder.get_seekable_stream_decoder_state();
				if(state__ == ::FLAC__SEEKABLE_STREAM_DECODER_STREAM_DECODER_ERROR)
					return decoder.get_stream_decoder_state().as_cstring();
				else
					return state__.as_cstring();
			}
			else
				return as_cstring();
		}

		File::File():
		decoder_(::FLAC__file_decoder_new())
		{ }

		File::~File()
		{
			if(0 != decoder_) {
				(void) ::FLAC__file_decoder_finish(decoder_);
				::FLAC__file_decoder_delete(decoder_);
			}
		}

		bool File::is_valid() const
		{
			return 0 != decoder_;
		}

		bool File::set_md5_checking(bool value)
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_md5_checking(decoder_, value);
		}

		bool File::set_filename(const char *value)
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_filename(decoder_, value);
		}

		bool File::set_metadata_respond(::FLAC__MetadataType type)
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_metadata_respond(decoder_, type);
		}

		bool File::set_metadata_respond_application(const FLAC__byte id[4])
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_metadata_respond_application(decoder_, id);
		}

		bool File::set_metadata_respond_all()
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_metadata_respond_all(decoder_);
		}

		bool File::set_metadata_ignore(::FLAC__MetadataType type)
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_metadata_ignore(decoder_, type);
		}

		bool File::set_metadata_ignore_application(const FLAC__byte id[4])
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_metadata_ignore_application(decoder_, id);
		}

		bool File::set_metadata_ignore_all()
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_set_metadata_ignore_all(decoder_);
		}

		File::State File::get_state() const
		{
			FLAC__ASSERT(0 != decoder_);
			return State(::FLAC__file_decoder_get_state(decoder_));
		}

		SeekableStream::State File::get_seekable_stream_decoder_state() const
		{
			FLAC__ASSERT(0 != decoder_);
			return SeekableStream::State(::FLAC__file_decoder_get_seekable_stream_decoder_state(decoder_));
		}

		Stream::State File::get_stream_decoder_state() const
		{
			FLAC__ASSERT(0 != decoder_);
			return Stream::State(::FLAC__file_decoder_get_stream_decoder_state(decoder_));
		}

		bool File::get_md5_checking() const
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_get_md5_checking(decoder_);
		}

		unsigned File::get_channels() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__file_decoder_get_channels(decoder_);
		}

		::FLAC__ChannelAssignment File::get_channel_assignment() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__file_decoder_get_channel_assignment(decoder_);
		}

		unsigned File::get_bits_per_sample() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__file_decoder_get_bits_per_sample(decoder_);
		}

		unsigned File::get_sample_rate() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__file_decoder_get_sample_rate(decoder_);
		}

		unsigned File::get_blocksize() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__file_decoder_get_blocksize(decoder_);
		}

		File::State File::init()
		{
			FLAC__ASSERT(0 != decoder_);
			::FLAC__file_decoder_set_write_callback(decoder_, write_callback_);
			::FLAC__file_decoder_set_metadata_callback(decoder_, metadata_callback_);
			::FLAC__file_decoder_set_error_callback(decoder_, error_callback_);
			::FLAC__file_decoder_set_client_data(decoder_, (void*)this);
			return State(::FLAC__file_decoder_init(decoder_));
		}

		bool File::finish()
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_finish(decoder_);
		}

		bool File::process_single()
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_process_single(decoder_);
		}

		bool File::process_until_end_of_metadata()
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_process_until_end_of_metadata(decoder_);
		}

		bool File::process_until_end_of_file()
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_process_until_end_of_file(decoder_);
		}

		bool File::seek_absolute(FLAC__uint64 sample)
		{
			FLAC__ASSERT(0 != decoder_);
			return (bool)::FLAC__file_decoder_seek_absolute(decoder_, sample);
		}

		::FLAC__StreamDecoderWriteStatus File::write_callback_(const ::FLAC__FileDecoder *decoder, const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
		{
			(void) decoder;
			FLAC__ASSERT(0 != client_data);
			File *instance = reinterpret_cast<File *>(client_data);
			FLAC__ASSERT(0 != instance);
			return instance->write_callback(frame, buffer);
		}

		void File::metadata_callback_(const ::FLAC__FileDecoder *decoder, const ::FLAC__StreamMetadata *metadata, void *client_data)
		{
			(void) decoder;
			FLAC__ASSERT(0 != client_data);
			File *instance = reinterpret_cast<File *>(client_data);
			FLAC__ASSERT(0 != instance);
			instance->metadata_callback(metadata);
		}

		void File::error_callback_(const ::FLAC__FileDecoder *decoder, ::FLAC__StreamDecoderErrorStatus status, void *client_data)
		{
			(void) decoder;
			FLAC__ASSERT(0 != client_data);
			File *instance = reinterpret_cast<File *>(client_data);
			FLAC__ASSERT(0 != instance);
			instance->error_callback(status);
		}

	};
};
