#include "sancu_sample.hh"
#include "sancu_signal.hh"

SancuSampleReader::SancuSampleReader(SancuSignal* const _signal) :
		signal(_signal), current_chunk(0)
{
}

std::vector<TSampleChunk> SancuSampleReader::read(const size_t& _num_chunks,
        const size_t& _last_chunk_size)
{
	std::vector<TSampleChunk> chunks;
	size_t total_chunks = signal->chunks.size();

	/* chunks pool need to be continuous, so if it is not,
	 * chunks need to be read from the beginning */
	if ((total_chunks - current_chunk) < _num_chunks
	        || signal->chunks[current_chunk + _num_chunks - 1]->length
	                < _last_chunk_size)
	{
		current_chunk = 0;
	}

	/* read chunks except last one */
	for (size_t i = 0; i < _num_chunks - 1; ++i)
	{
		chunks.push_back(
		        TSampleChunk(signal->chunks[current_chunk],
		                SancuSignalChunk::BUF_LEN));
		current_chunk = (current_chunk + 1) % total_chunks;
	}

	/* read last chunk */
	chunks.push_back(
	        TSampleChunk(signal->chunks[current_chunk], _last_chunk_size));

	return chunks;
}

SancuSample::SancuSample(std::vector<TSampleChunk>& _chunks) : chunks(_chunks)
{
	compute_energy();
}

void SancuSample::compute_energy()
{
	std::vector<TSampleChunk>::iterator iter = chunks.begin();

	for (; iter != chunks.end(); ++iter)
	{
		energy += ::compute_energy(iter->first->buffer, iter->second);
	}
}
