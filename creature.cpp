#include"funcs.c"
#include"random.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <unordered_map>

#define NEXT_ID() (__COUNTER__)

//keep this always const
class Gene {
public:
    virtual ~Gene() = default;
    virtual constexpr inline int getID() const = 0;
    virtual bit_c_t bigest_encode_memory(bit_c_t size) const = 0;
    virtual bit_c_t bigest_decode_memory(bit_c_t size) const = 0;
    virtual std::unique_ptr<Gene> mutatate(pcg32_random_t* rng) const = 0;
    //virtual void printData() const = 0;

    virtual void serialize(std::ostream& os) const = 0;
    virtual bit_c_t encode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const =0;
    virtual bit_c_t decode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const =0;

    static std::unique_ptr<const Gene> deserialize(std::istream& is); 
};

template <int UniqueIDValue>
class IDGene : public Gene {
public:
    static constexpr int ID = UniqueIDValue;
    constexpr inline int getID() const override {
        return ID;
    }
    //static virtual std::unique_ptr<Gene> subclass_derialize(std::istream& is)
    virtual void subclass_serialize(std::ostream& os) const = 0;
    void serialize(std::ostream& os) const override{
    	os << ID;
    	subclass_serialize(os);
    }
};

#define max_rle_window 60000
#define min_rle_window 1
#define rle_mutate 3
class RLE_Gene : public IDGene<NEXT_ID()>{
private:
	const window_t window;
public:
	RLE_Gene( window_t w) : window(w) {}

	bit_c_t encode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const override{
		return RLE_encode(in,start,end,out,pos,window);
	}

	bit_c_t decode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const override{
		return RLE_decode(in,start,end,out,pos,window);
	}

	std::unique_ptr<Gene> mutatate(pcg32_random_t* rng) const override{
		int w=pcg32_random_r(rng)%(2*rle_mutate);
		w-=rle_mutate;
		w+=window;

		w = std::max(max_rle_window,w);
		w = std::min(min_rle_window,w);
		return std::make_unique<RLE_Gene>(w);
	}

	bit_c_t bigest_encode_memory(bit_c_t size) const override{
		return size+size/window;
	}

	bit_c_t bigest_decode_memory(bit_c_t size) const override{
		return size*window;
	}

	void subclass_serialize(std::ostream& os) const override{
		os<<window;
	}
	static std::unique_ptr<const Gene> subclass_derialize(std::istream& is) {
		window_t w;
		is >>w;
		return std::make_unique<RLE_Gene>(w);
	}
};

class XOR_Gene : public IDGene<NEXT_ID()>{
private:
	const window_t state;
	const window_t increment; 
	const window_t mul;
	const window_t period;

	bit_c_t my_xor(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const{
		XOR( in,start, end, out, pos,state,increment,mul,period);
		return end-start;
	}
public:
	XOR_Gene(window_t state_, window_t increment_, window_t mul_, window_t period_)
        : state(state_), increment(increment_), mul(mul_), period(period_) {}

	bit_c_t encode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const override{
		return my_xor(in,start,end,out,pos);
	}
	bit_c_t decode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const override{
		return my_xor(in,start,end,out,pos);
	}
	bit_c_t bigest_encode_memory(bit_c_t size) const override{
		return size;
	}

	bit_c_t bigest_decode_memory(bit_c_t size) const override{
		return size;
	}
	void subclass_serialize(std::ostream& os) const override{
		os<<state<<increment<<mul<<period;
	}
	static std::unique_ptr<const Gene> subclass_derialize(std::istream& is) {
		window_t state; window_t increment; window_t mul; window_t period;
		is >>state>>increment>>mul>>period;
		return std::make_unique<XOR_Gene>(state,increment,mul,period);
	}
};

std::unordered_map<int, std::unique_ptr<const Gene>(*)(std::istream&)> deserializers={
    {RLE_Gene::ID, RLE_Gene::subclass_derialize},

};

std::unique_ptr<const Gene> Gene::deserialize(std::istream& is) {
        int id;
        is >> id;
        auto it = deserializers.find(id);
        if (it == deserializers.end()) {
            throw std::runtime_error("Unknown Gene ID");
        }
        return it->second(is);
    }



class Creature {
public:
	std::unique_ptr<const std::shared_ptr<const Gene>[]> genes;  // Array of shared_ptrs
    const size_t num_genes;

    Creature(std::unique_ptr<std::shared_ptr<const Gene>[]> genesArray, size_t n)
        : genes(std::move(genesArray)), num_genes(n) {}

    void encode(bit_buffer_t** in){
    	if(!num_genes){
    		return;
    	}
    	bit_buffer_t* a=*in;
    	bit_buffer_t* b=init_buffer(genes[0]->bigest_encode_memory(a->size));
    	bit_c_t size=genes[0]->encode(a,0,a->size,b,0); 

    	for(int i=1;i<num_genes;i++){
    		std::swap(a,b);
    		reserve_buffer(b,genes[i]->bigest_encode_memory(size));
    		 size=genes[i]->encode(a,0,size,b,0);
    	} 
    	free_buffer(a);
    	resize_buffer(b,size);
    	*in=b;
    }

    void decode(bit_buffer_t** in){
    	if(!num_genes){
    		return;
    	}
    	bit_buffer_t* a=*in;
    	bit_buffer_t* b=init_buffer(genes[num_genes-1]->bigest_decode_memory(a->size));
    	bit_c_t size=genes[num_genes-1]->decode(a,0,a->size,b,0); 

    	for(int i=num_genes-2;i>=0;i--){
    		std::swap(a,b);
    		reserve_buffer(b,genes[i]->bigest_decode_memory(size));
    		 size=genes[i]->decode(a,0,size,b,0);
    	} 
    	free_buffer(a);
    	resize_buffer(b,size);
    	*in=b;
    }
	void save(const std::string& filename) const {
	    std::ofstream ofs(filename);
	    if (!ofs.is_open()) {
	        throw std::runtime_error("Failed to open file for saving");
	    }

	    // Write the number of genes
	    ofs << num_genes << std::endl;

	    // Serialize each gene
	    for (size_t i = 0; i < num_genes; i++) {
	        genes[i]->serialize(ofs);
	        ofs << std::endl;
	    }
	}


    static Creature load(const std::string& filename) {
	    std::ifstream ifs(filename);
	    if (!ifs.is_open()) {
	        throw std::runtime_error("Failed to open file for loading");
	    }

	    // Read the number of genes
	    size_t numGenes;
	    ifs >> numGenes;

	    // Create a temporary array of shared_ptrs
	    auto tempGenes = std::make_unique<std::shared_ptr<const Gene>[]>(numGenes);

	    // Populate the array
	    for (size_t i = 0; i < numGenes; ++i) {
	        tempGenes[i] = Gene::deserialize(ifs);
	    }

	    // Construct the Creature using the populated array
	    return Creature(std::move(tempGenes), numGenes);
	}

};

