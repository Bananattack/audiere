#include "input_wav.hpp"
#include "endian.hpp"
#include "utility.hpp"


static inline adr_u16 Read16(adr_u8* m) {
  return (adr_u16)(
    (m[0] << 0) +
    (m[1] << 8)
  );
}

static inline adr_u32 Read32(adr_u8* m) {
  return (adr_u32)(
    (m[0] << 0) +
    (m[1] << 8) +
    (m[2] << 16)  +
    (m[3] << 24)
  );
}

static inline bool IsValidSampleSize(adr_u32 size) {
  return (size == 8 || size == 16);
}

////////////////////////////////////////////////////////////////////////////////

WAVInputStream::WAVInputStream()
{
  m_file = 0;

  m_channel_count   = 0;
  m_bits_per_sample = 0;
  m_sample_rate     = 0;

  m_data_chunk_location = 0;
  m_data_chunk_length   = 0;

  m_samples_left_in_chunk = 0;
}

////////////////////////////////////////////////////////////////////////////////

WAVInputStream::~WAVInputStream()
{
  delete m_file;
  m_file = 0;
}

////////////////////////////////////////////////////////////////////////////////

bool
WAVInputStream::Initialize(IFile* file)
{
  m_file = file;

  // read the RIFF header
  char    riff_id[4];
  adr_u32 riff_length;
  char    riff_datatype[4];

  adr_u32 size = 0;
  size += file->Read(riff_id, 4);
  size += file->Read(&riff_length, 4);
  size += file->Read(riff_datatype, 4);

  riff_length = LittleToHost32(riff_length);

  if (size != 12 ||
      memcmp(riff_id, "RIFF", 4) != 0 ||
      riff_length == 0 ||
      memcmp(riff_datatype, "WAVE", 4) != 0) {

    // so we don't destroy the file
    m_file = 0;
    return false;
  }

  if (FindFormatChunk() && FindDataChunk()) {
    return true;
  } else {
    m_file = 0;
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void
WAVInputStream::GetFormat(
  int& channel_count,
  int& sample_rate,
  int& bits_per_sample)
{
  channel_count   = m_channel_count;
  sample_rate     = m_sample_rate;
  bits_per_sample = m_bits_per_sample;
}

////////////////////////////////////////////////////////////////////////////////

int
WAVInputStream::Read(int sample_count, void* samples)
{
  if (m_samples_left_in_chunk == 0) {
    return 0;
  }

  const int samples_to_read = adr_min(sample_count, m_samples_left_in_chunk);
  const int sample_size = m_channel_count * m_bits_per_sample / 8;
  const int bytes_to_read = samples_to_read * sample_size;
  
  const int read = m_file->Read(samples, bytes_to_read);

  // assume that if we didn't get a full read, we're done
  if (read != bytes_to_read) {
    m_samples_left_in_chunk = 0;
    return read / sample_size;
  }

  m_samples_left_in_chunk -= read / sample_size;

  return sample_count;
}

////////////////////////////////////////////////////////////////////////////////

bool
WAVInputStream::Reset()
{
  // seek to the beginning of the data chunk
  m_samples_left_in_chunk = m_data_chunk_length;
  return m_file->Seek(m_data_chunk_location, ADR_BEGIN);
}

////////////////////////////////////////////////////////////////////////////////

bool
WAVInputStream::FindFormatChunk()
{
  // seek to just after the RIFF header
  m_file->Seek(12, ADR_BEGIN);

  // search for a format chunk
  while (true) {
    char    chunk_id[4];
    adr_u32 chunk_length;

    int size = m_file->Read(chunk_id, 4);
    size    += m_file->Read(&chunk_length, 4);
    chunk_length = LittleToHost32(chunk_length);

    // if we couldn't read enough, we're done
    if (size != 8) {
      return false;
    }

    // if we found a format chunk, excellent!
    if (memcmp(chunk_id, "fmt ", 4) == 0 && chunk_length >= 16) {

      // read format chunk
      adr_u8 chunk[16];
      size = m_file->Read(chunk, 16);

      // could we read the entire format chunk?
      if (size < 16) {
        return false;
      }

      chunk_length -= size;

      // parse the memory into useful information
      adr_u16 format_tag         = Read16(chunk + 0);
      adr_u16 channel_count      = Read16(chunk + 2);
      adr_u32 samples_per_second = Read32(chunk + 4);
      adr_u32 bytes_per_second   = Read32(chunk + 8);
      adr_u16 block_align        = Read16(chunk + 12);
      adr_u16 bits_per_sample    = Read16(chunk + 14);

      // format_tag must be 1 (WAVE_FORMAT_PCM)
      // we only support mono and stereo
      if (format_tag != 1 ||
          channel_count > 2 ||
          !IsValidSampleSize(bits_per_sample)) {
        return false;
      }

      // skip the rest of the chunk
      if (!SkipBytes(chunk_length)) {
        // oops, end of stream
        return false;
      }

      // set format
      m_channel_count   = channel_count;
      m_bits_per_sample = bits_per_sample;
      m_sample_rate     = samples_per_second;
      return true;

    } else {

      // skip the rest of the chunk
      if (!SkipBytes(chunk_length)) {
        // oops, end of stream
        return false;
      }

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
WAVInputStream::FindDataChunk()
{
  // seek to just after the RIFF header
  m_file->Seek(12, ADR_BEGIN);

  // search for a format chunk
  while (true) {
    char    chunk_id[4];
    adr_u32 chunk_length;

    int size = m_file->Read(chunk_id, 4);
    size    += m_file->Read(&chunk_length, 4);
    chunk_length = LittleToHost32(chunk_length);

    // if we couldn't read enough, we're done
    if (size != 8) {
      return false;
    }

    // if we found a data chunk, excellent!
    if (memcmp(chunk_id, "data", 4) == 0) {

      // calculate the sample size so we can truncate the data chunk
      int sample_size = m_channel_count * m_bits_per_sample / 8;

      m_data_chunk_location   = m_file->Tell();
      m_data_chunk_length     = chunk_length / sample_size;
      m_samples_left_in_chunk = m_data_chunk_length;
      return true;

    } else {

      // skip the rest of the chunk
      if (!SkipBytes(chunk_length)) {
        // oops, end of stream
        return false;
      }

    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool
WAVInputStream::SkipBytes(int size)
{
  if (m_file->Seek(size, ADR_CURRENT)) {
    m_samples_left_in_chunk = m_data_chunk_length;
    return true;
  } else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
