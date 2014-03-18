/*
 * sancu_sample.hh
 *
 *  Created on: Mar 18, 2014
 *      Author: norethel
 */

#ifndef SANCU_SAMPLE_HH_
#define SANCU_SAMPLE_HH_

#include <vector>
#include <cstddef>

class SancuSignal;
class SancuSignalChunk;

typedef std::pair<SancuSignalChunk*, size_t> TSampleChunk;

class SancuSampleReader
{
	public:

	SancuSampleReader(SancuSignal* const _signal);

	std::vector<TSampleChunk> read(const size_t& _num_chunks,
	        const size_t& _last_chunk_size);

	private:

	SancuSignal* signal;
	size_t current_chunk;
};

class SancuSample
{
	public:

	double energy;
	std::vector<TSampleChunk> chunks;

	SancuSample(std::vector<TSampleChunk>& _chunks);

	private:

	void compute_energy();
};

#endif /* SANCU_SAMPLE_HH_ */
