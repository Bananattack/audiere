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

#include "FLAC++/encoder.h"
#include "FLAC/assert.h"

namespace FLAC {
	namespace Encoder {

		const char *SeekableStream::State::resolved_as_cstring(const SeekableStream &encoder) const
		{
			if(state_ == ::FLAC__SEEKABLE_STREAM_ENCODER_STREAM_ENCODER_ERROR) {
				FLAC::Encoder::Stream::State state__ = encoder.get_stream_encoder_state();
				if(state__ == ::FLAC__STREAM_ENCODER_VERIFY_DECODER_ERROR)
					return encoder.get_verify_decoder_state().as_cstring();
				else
					return state__.as_cstring();
			}
			else
				return as_cstring();
		}


		SeekableStream::SeekableStream():
		encoder_(::FLAC__seekable_stream_encoder_new())
		{ }

		SeekableStream::~SeekableStream()
		{
			if(0 != encoder_) {
				::FLAC__seekable_stream_encoder_finish(encoder_);
				::FLAC__seekable_stream_encoder_delete(encoder_);
			}
		}

		bool SeekableStream::is_valid() const
		{
			return 0 != encoder_;
		}

		bool SeekableStream::set_verify(bool value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_verify(encoder_, value);
		}

		bool SeekableStream::set_streamable_subset(bool value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_streamable_subset(encoder_, value);
		}

		bool SeekableStream::set_do_mid_side_stereo(bool value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_do_mid_side_stereo(encoder_, value);
		}

		bool SeekableStream::set_loose_mid_side_stereo(bool value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_loose_mid_side_stereo(encoder_, value);
		}

		bool SeekableStream::set_channels(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_channels(encoder_, value);
		}

		bool SeekableStream::set_bits_per_sample(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_bits_per_sample(encoder_, value);
		}

		bool SeekableStream::set_sample_rate(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_sample_rate(encoder_, value);
		}

		bool SeekableStream::set_blocksize(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_blocksize(encoder_, value);
		}

		bool SeekableStream::set_max_lpc_order(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_max_lpc_order(encoder_, value);
		}

		bool SeekableStream::set_qlp_coeff_precision(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_qlp_coeff_precision(encoder_, value);
		}

		bool SeekableStream::set_do_qlp_coeff_prec_search(bool value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_do_qlp_coeff_prec_search(encoder_, value);
		}

		bool SeekableStream::set_do_escape_coding(bool value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_do_escape_coding(encoder_, value);
		}

		bool SeekableStream::set_do_exhaustive_model_search(bool value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_do_exhaustive_model_search(encoder_, value);
		}

		bool SeekableStream::set_min_residual_partition_order(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_min_residual_partition_order(encoder_, value);
		}

		bool SeekableStream::set_max_residual_partition_order(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_max_residual_partition_order(encoder_, value);
		}

		bool SeekableStream::set_rice_parameter_search_dist(unsigned value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_rice_parameter_search_dist(encoder_, value);
		}

		bool SeekableStream::set_total_samples_estimate(FLAC__uint64 value)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_total_samples_estimate(encoder_, value);
		}

		bool SeekableStream::set_metadata(::FLAC__StreamMetadata **metadata, unsigned num_blocks)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_set_metadata(encoder_, metadata, num_blocks);
		}

		SeekableStream::State SeekableStream::get_state() const
		{
			FLAC__ASSERT(is_valid());
			return State(::FLAC__seekable_stream_encoder_get_state(encoder_));
		}

		Stream::State SeekableStream::get_stream_encoder_state() const
		{
			FLAC__ASSERT(is_valid());
			return Stream::State(::FLAC__seekable_stream_encoder_get_stream_encoder_state(encoder_));
		}

		Decoder::Stream::State SeekableStream::get_verify_decoder_state() const
		{
			FLAC__ASSERT(is_valid());
			return Decoder::Stream::State(::FLAC__seekable_stream_encoder_get_verify_decoder_state(encoder_));
		}

		void SeekableStream::get_verify_decoder_error_stats(FLAC__uint64 *absolute_sample, unsigned *frame_number, unsigned *channel, unsigned *sample, FLAC__int32 *expected, FLAC__int32 *got)
		{
			FLAC__ASSERT(is_valid());
			::FLAC__seekable_stream_encoder_get_verify_decoder_error_stats(encoder_, absolute_sample, frame_number, channel, sample, expected, got);
		}

		bool SeekableStream::get_verify() const
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_get_verify(encoder_);
		}

		bool SeekableStream::get_streamable_subset() const
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_get_streamable_subset(encoder_);
		}

		bool SeekableStream::get_do_mid_side_stereo() const
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_get_do_mid_side_stereo(encoder_);
		}

		bool SeekableStream::get_loose_mid_side_stereo() const
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_get_loose_mid_side_stereo(encoder_);
		}

		unsigned SeekableStream::get_channels() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_channels(encoder_);
		}

		unsigned SeekableStream::get_bits_per_sample() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_bits_per_sample(encoder_);
		}

		unsigned SeekableStream::get_sample_rate() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_sample_rate(encoder_);
		}

		unsigned SeekableStream::get_blocksize() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_blocksize(encoder_);
		}

		unsigned SeekableStream::get_max_lpc_order() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_max_lpc_order(encoder_);
		}

		unsigned SeekableStream::get_qlp_coeff_precision() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_qlp_coeff_precision(encoder_);
		}

		bool SeekableStream::get_do_qlp_coeff_prec_search() const
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_get_do_qlp_coeff_prec_search(encoder_);
		}

		bool SeekableStream::get_do_escape_coding() const
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_get_do_escape_coding(encoder_);
		}

		bool SeekableStream::get_do_exhaustive_model_search() const
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_get_do_exhaustive_model_search(encoder_);
		}

		unsigned SeekableStream::get_min_residual_partition_order() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_min_residual_partition_order(encoder_);
		}

		unsigned SeekableStream::get_max_residual_partition_order() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_max_residual_partition_order(encoder_);
		}

		unsigned SeekableStream::get_rice_parameter_search_dist() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_rice_parameter_search_dist(encoder_);
		}

		FLAC__uint64 SeekableStream::get_total_samples_estimate() const
		{
			FLAC__ASSERT(is_valid());
			return ::FLAC__seekable_stream_encoder_get_total_samples_estimate(encoder_);
		}

		SeekableStream::State SeekableStream::init()
		{
			FLAC__ASSERT(is_valid());
			::FLAC__seekable_stream_encoder_set_seek_callback(encoder_, seek_callback_);
			::FLAC__seekable_stream_encoder_set_write_callback(encoder_, write_callback_);
			::FLAC__seekable_stream_encoder_set_client_data(encoder_, (void*)this);
			return State(::FLAC__seekable_stream_encoder_init(encoder_));
		}

		void SeekableStream::finish()
		{
			FLAC__ASSERT(is_valid());
			::FLAC__seekable_stream_encoder_finish(encoder_);
		}

		bool SeekableStream::process(const FLAC__int32 * const buffer[], unsigned samples)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_process(encoder_, buffer, samples);
		}

		bool SeekableStream::process_interleaved(const FLAC__int32 buffer[], unsigned samples)
		{
			FLAC__ASSERT(is_valid());
			return (bool)::FLAC__seekable_stream_encoder_process_interleaved(encoder_, buffer, samples);
		}

		::FLAC__SeekableStreamEncoderSeekStatus SeekableStream::seek_callback_(const ::FLAC__SeekableStreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data)
		{
			(void)encoder;
			FLAC__ASSERT(0 != client_data);
			SeekableStream *instance = reinterpret_cast<SeekableStream *>(client_data);
			FLAC__ASSERT(0 != instance);
			return instance->seek_callback(absolute_byte_offset);
		}

		::FLAC__StreamEncoderWriteStatus SeekableStream::write_callback_(const ::FLAC__SeekableStreamEncoder *encoder, const FLAC__byte buffer[], unsigned bytes, unsigned samples, unsigned current_frame, void *client_data)
		{
			(void)encoder;
			FLAC__ASSERT(0 != client_data);
			SeekableStream *instance = reinterpret_cast<SeekableStream *>(client_data);
			FLAC__ASSERT(0 != instance);
			return instance->write_callback(buffer, bytes, samples, current_frame);
		}

	};
};
