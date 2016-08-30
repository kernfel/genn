
#define DT 0.1
bool switchTypeOfSparse = true;


#include "modelSpec.h"
#include "modelSpec.cc"
double p__Population[0]={
		};
double ini__Population[3]={
			0, // voltage for triggering spikes
0.0, //
		1, // initial regime
};
double p__PopulationS1[4]={
		10, // 0 - tau
		0.5, // 1 - m
		0, // 2 - c
		100, // 3 - tau_adap
		};
double ini__PopulationS1[5]={
			0, // voltage for triggering spikes
0, // 0 - a
		0, // 1 - adap
		0.0, //
		1, // initial regime
};
void modelDefinition(NNmodel &model) 
{
	#include "extra_neurons.h"
	#include "extra_postsynapses.h"
	#include "extra_weightupdates.h"
	POISSONNEURON = INT_MAX;
		
	initGeNN();
	model.setGPUDevice(0); 
	model.setName("Untitled_Project");
	model.addNeuronPopulation("Population", 3, 0+0, p__Population, ini__Population);
	model.addNeuronPopulation("PopulationS1", 3, 0+1, p__PopulationS1, ini__PopulationS1);

double genInputSV[1] = {0};

	
	model.addSynapsePopulation("Input1_Population_to_PopulationS1",0+0, DENSE, GLOBALG, NO_DELAY, 0+0, "Population", "PopulationS1", /*genInputSV*/ NULL, NULL, NULL, NULL);
	//model.setSynapseG("Synapse1_PopulationS1_to_",	0);
	model.setPrecision(0);
	model.finalize();
}
	
