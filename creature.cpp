extern "C" {
    #include"funcs.c"
	#include"random.h"
}

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <unordered_map>

#define NEXT_ID() (__COUNTER__)

static int perturbation(pcg32_random_t* rng,int input,int range,int min,int max){
	int w=pcg32_random_r(rng)%(2*range);
	w-=range;
	w+=input;

	w = std::min(max,w);
	w = std::max(min,w);
	return w;
}


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
    static std::unique_ptr<const Gene> random_gene(pcg32_random_t* rng);
};

template <int UniqueIDValue>
class IDGene : public Gene {
public:
    static constexpr int ID = UniqueIDValue;
    constexpr inline int getID() const override {
        return ID;
    }
    //static virtual std::unique_ptr<Gene> subclass_deserialize(std::istream& is)
    virtual void subclass_serialize(std::ostream& os) const = 0;
    void serialize(std::ostream& os) const override{
    	os << ID;
    	subclass_serialize(os);
    }
};

#define MAX_RLE_WINDOW 8*4*10
#define MIN_RLE_WINDOW 1
#define RLE_MUTATE 3
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
		window_t w= perturbation(rng,window,RLE_MUTATE, MIN_RLE_WINDOW, MAX_RLE_WINDOW);
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
	static std::unique_ptr<const Gene> subclass_deserialize(std::istream& is) {
		window_t w;
		is >>w;
		return std::make_unique<RLE_Gene>(w);
	}
};

#define XOR_STATE_MUTATION_CHANCE 60

#define MAX_XOR_PERIOD 8*4*7
#define MIN_XOR_PERIOD 2
#define XOR_PERIOD_MUTATE 3
#define XOR_PERIOD_MUTATE_CHANCE 30

#define MAX_XOR_INCREMENT 1<<16-1
#define MIN_XOR_INCREMENT 1
#define XOR_INCREMENT_MUTATE 29
#define XOR_INCREMENT_MUTATE_CHANCE 30

#define MAX_XOR_MUL 1<<16-1
#define MIN_XOR_MUL 1
#define XOR_MUL_MUTATE 20
#define XOR_MUL_MUTATE_CHANCE 10


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


	std::unique_ptr<Gene> mutatate(pcg32_random_t* rng) const override{
		window_t p, i, m, s;
	    if ((pcg32_random_r(rng) % 100) < XOR_PERIOD_MUTATE_CHANCE) {
	        p = perturbation(rng, period, XOR_PERIOD_MUTATE, MIN_XOR_PERIOD, MAX_XOR_PERIOD);
	    } else {
	        p = period;
	    }

	    if ((pcg32_random_r(rng) % 100) < XOR_INCREMENT_MUTATE_CHANCE) {
	        i = perturbation(rng, increment, XOR_INCREMENT_MUTATE, MIN_XOR_INCREMENT, MAX_XOR_INCREMENT);
	    } else {
	        i = increment;
	    }
	    
	    if ((pcg32_random_r(rng) % 100) < XOR_MUL_MUTATE_CHANCE) {
	        m = perturbation(rng, mul, XOR_MUL_MUTATE, MIN_XOR_MUL, MAX_XOR_MUL);
	    } else {
	        m = mul;
	    }

	    if ((pcg32_random_r(rng) % 100) < XOR_STATE_MUTATION_CHANCE){
	    	s = (state | pcg32_random_r(rng)) & pcg32_random_r(rng);
	    }
	    else{
	    	s=state;
	    }

	    return std::make_unique<XOR_Gene>(s, i, m, p);
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
	static std::unique_ptr<const Gene> subclass_deserialize(std::istream& is) {
		window_t state; window_t increment; window_t mul; window_t period;
		is >>state>>increment>>mul>>period;
		return std::make_unique<XOR_Gene>(state,increment,mul,period);
	}
};

#define MAX_PERM_WINDOW 8*4*5
#define MIN_PERM_WINDOW 1
#define PERM_MUTATION 3

#define PERM_WINDOW_MUTATION_CHANCE 30
#define PERM_MAX_MUTATION_CHANCE 30
#define PERM_MIN_MUTATION_CHANCE 30

class PERM_Gene : public IDGene<NEXT_ID()>{
private:
	const window_t window;
	const window_t max; 
	const window_t min;

	bit_c_t my_perm(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const{
		CONDITIONAL_PERMUTE( in, start, end, out, pos, window, max, min);
		return end-start;
	}
public:
	PERM_Gene(window_t window_, window_t max_, window_t min_)
        : window(window_), max(max_), min(min_) {}
	bit_c_t encode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const override{
		return my_perm(in,start,end,out,pos);
	}
	bit_c_t decode(bit_buffer_t* in,bit_c_t start,bit_c_t end,bit_buffer_t* out,bit_c_t pos) const override{
		return my_perm(in,start,end,out,pos);
	}
	std::unique_ptr<Gene> mutatate(pcg32_random_t* rng) const override{
		window_t w,max_,min_;

		if((pcg32_random_r(rng) % 100)<PERM_WINDOW_MUTATION_CHANCE){
			w=perturbation(rng,window,PERM_MUTATION,MIN_PERM_WINDOW,MAX_PERM_WINDOW);
		}
		else{
			w=window;
		}

		if((pcg32_random_r(rng) % 100)<PERM_MAX_MUTATION_CHANCE){
			max_=perturbation(rng,max,PERM_MUTATION,MIN_PERM_WINDOW,MAX_PERM_WINDOW);
		}
		else{
			max_=max;
		}

		if((pcg32_random_r(rng) % 100)<PERM_MIN_MUTATION_CHANCE){
			min_=perturbation(rng,min,PERM_MUTATION,MIN_PERM_WINDOW,MAX_PERM_WINDOW);
		}
		else{
			min_=min;
		}

		return std::make_unique<PERM_Gene>(w,max_,min_);
	}

	bit_c_t bigest_encode_memory(bit_c_t size) const override{
		return size;
	}

	bit_c_t bigest_decode_memory(bit_c_t size) const override{
		return size;
	}
	void subclass_serialize(std::ostream& os) const override{
		os<<window<<max<<min;
	}
	static std::unique_ptr<const Gene> subclass_deserialize(std::istream& is) {
		window_t window; window_t max; window_t min;
		is >>window>>max>>min;
		return std::make_unique<PERM_Gene>(window,max,min);
	}
};


std::unordered_map<int, std::unique_ptr<const Gene>(*)(std::istream&)> Gene_Deserializers={
    {RLE_Gene::ID, RLE_Gene::subclass_deserialize},
    {XOR_Gene::ID, XOR_Gene::subclass_deserialize},
    {PERM_Gene::ID, PERM_Gene::subclass_deserialize},

};



std::unique_ptr<const Gene> Gene::deserialize(std::istream& is) {
        int id;
        is >> id;
        auto it = Gene_Deserializers.find(id);
        if (it == Gene_Deserializers.end()) {
            throw std::runtime_error("Unknown Gene ID");
        }
        return it->second(is);
    }

#define RLE_CHANCE 30
#define XOR_CHANCE 30
#define PERM_CHANCE 20
#define TOTAL_CHANCE RLE_CHANCE+XOR_CHANCE+PERM_CHANCE 

// Constants for sampling
#define RLE_WINDOW_SAMPLE_COUNT 3

#define XOR_STATE_SAMPLE_COUNT 3
#define XOR_INCREMENT_SAMPLE_COUNT 3
#define XOR_MUL_SAMPLE_COUNT 3
#define XOR_PERIOD_SAMPLE_COUNT 3

#define PERM_WINDOW_SAMPLE_COUNT 3

static int value_gen(pcg32_random_t* rng, int min, int max, int sample_count) {
    int x = 0;
    for(int i = 0; i < sample_count; i++) {
        x += pcg32_random_r(rng) / sample_count;
    }
    return (x % (max - min + 1)) + min;
}

std::unique_ptr<const Gene> Gene::random_gene(pcg32_random_t* rng){
    int t = pcg32_random_r(rng) % TOTAL_CHANCE;

    if(t < RLE_CHANCE){
        return std::make_unique<RLE_Gene>(value_gen(rng, MIN_RLE_WINDOW, MAX_RLE_WINDOW, RLE_WINDOW_SAMPLE_COUNT));
    }
    t -= RLE_CHANCE;
    
    if(t < XOR_CHANCE){
        return std::make_unique<XOR_Gene>(
            pcg32_random_r(rng),
            value_gen(rng, MIN_XOR_INCREMENT, MAX_XOR_PERIOD, XOR_INCREMENT_SAMPLE_COUNT),
            value_gen(rng, MIN_XOR_MUL, MAX_XOR_PERIOD, XOR_MUL_SAMPLE_COUNT),
            value_gen(rng, MIN_XOR_PERIOD, MAX_XOR_PERIOD, XOR_PERIOD_SAMPLE_COUNT)
        );
    }

    // For PERM_Gene
	int perm_window = value_gen(rng, MIN_PERM_WINDOW, MAX_PERM_WINDOW, PERM_WINDOW_SAMPLE_COUNT);
	int perm_min = value_gen(rng, MIN_PERM_WINDOW, perm_window, PERM_WINDOW_SAMPLE_COUNT);
	int perm_max = value_gen(rng, perm_window, MAX_PERM_WINDOW, PERM_WINDOW_SAMPLE_COUNT);

	return std::make_unique<PERM_Gene>(perm_window, perm_min, perm_max);

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

