/*--------------------------------------------------------------------------
   Author: Thomas Nowotny
  
   Institute: Center for Computational Neuroscience and Robotics
              University of Sussex
	      Falmer, Brighton BN1 9QJ, UK 
  
   email to:  T.Nowotny@sussex.ac.uk
  
   initial version: 2010-02-07
  
--------------------------------------------------------------------------*/

//-----------------------------------------------------------------------
/*!  \file generateHost.cc 
  
  \brief Contains functions to generate code for running the simulation on 
  the CPU, and host functions for synchronising GPU device kernels.
*/
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
/*!
  \brief A function that generates host-side code.

  In this function host-side functions and other code are generated,
  including: Global host variables, "allocatedMemHost()" function for
  allocating memories, "freeMemHost" function for freeing the allocated
  memories, "initializeHost" for initializing host variables, "gFuncHost" and
  "initGRawHost()" for use with plastic synapses if such synapses exist in
  the model.  
*/
//--------------------------------------------------------------------------

void genHostCode(ostream &mos)
{
  //mos << "entering genHostCode" << endl;

  string ccFile, hFile;
  ofstream os, osh;
  unsigned int type, tmp, size, mem = 0;


  //----------------------
  // open and setup host.h

  hFile = path + tS("/") + model->name + tS("_CODE_HOST/host.h");
  osh.open(hFile.c_str());
  writeHeader(osh);
  osh << "//-------------------------------------------------------------------------" << endl;
  osh << "/*! \\file host.h" << endl << endl;
  osh << "\\brief File generated by GeNN for the model " << model->name << " containing host declarations." << endl;
  osh << "*/" << endl;
  osh << "//-------------------------------------------------------------------------" << endl << endl;
  osh << endl;


  //------------------------------
  // declare host neuron variables

  osh << "// neuron variable declarations" << endl;
  for (int i = 0; i < model->neuronGrpN; i++) {
    type = model->neuronType[i];
    osh << "extern unsigned int glbscnt" << model->neuronName[i] << ";" << endl;
    osh << "extern unsigned int *glbSpk" << model->neuronName[i] << ";" << endl;
    if (model->neuronDelaySlots[i] == 1) {
      osh << "extern unsigned int glbSpkEvntCnt" << model->neuronName[i] << ";" << endl;
    }
    else {
      osh << "extern unsigned int spkEvntQuePtr" << model->neuronName[i] << ";" << endl;
      osh << "extern unsigned int *glbSpkEvntCnt" << model->neuronName[i] << ";" << endl;
    }
    osh << "extern unsigned int *glbSpkEvnt" << model->neuronName[i] << ";" << endl;
    for (int j = 0; j < model->inSyn[i].size(); j++) {
      osh << "extern " << model->ftype << " *inSyn" << model->neuronName[i] << j << ";" << endl;
    } 
    for (int j = 0; j < nModels[type].varNames.size(); j++) {
      osh << "extern " << nModels[type].varTypes[j] << " *" << nModels[type].varNames[j];
      osh << model->neuronName[i] << ";" << endl;
    }
    if (model->neuronNeedSt[i]) {
      osh << "extern " << model->ftype << " *sT" << model->neuronName[i] << ";" << endl;
    }
  }
  osh << endl;


  //------------------------------
  // declare host synase variables

  osh << "// synapse variable declarations" << endl;
  for (int i = 0; i < model->postSynapseType.size(); i++) {
    type = model->postSynapseType[i];
    for (int j = 0; j < postSynModels[type].varNames.size(); j++) {
      osh << "extern " << postSynModels[type].varTypes[j] << " *";
      osh << postSynModels[type].varNames[j] << model->synapseName[i] << ";" << endl;
    }
  }
  osh << "struct Conductance {" << endl;
  osh << "  " << model->ftype << " *gp;" << endl; // only if !globalg
  osh << "  unsigned int *gIndInG;" << endl;
  osh << "  unsigned int *gInd;" << endl;
  osh << "  unsigned int connN;" << endl; 
  osh << "};" << endl;
  for (int i = 0; i < model->synapseGrpN; i++) {
    if (model->synapseConnType[i] == SPARSE) {
      osh << "extern Conductance g" << model->synapseName[i] << ";" << endl;
    }
    else {
      if (model->synapseGType[i] == INDIVIDUALG) {
	osh << "extern " << model->ftype << " *gp" << model->synapseName[i] << ";" << endl;
      }
      if (model->synapseGType[i] == INDIVIDUALID) {
      	osh << "extern unsigned int *gp" << model->synapseName[i] << ";" << endl;
      }	 
    }
    if (model->synapseType[i] == LEARN1SYNAPSE) {
      osh << "extern " << model->ftype << " *grawp" << model->synapseName[i] << ";" << endl;
    }
  }
  osh << endl;
  osh << "// host function declarations" << endl;


  //-----------------------
  // open and setup host.cc

  ccFile = path + tS("/") + model->name + tS("_CODE_HOST/host.cc");
  os.open(ccFile.c_str());
  writeHeader(os);
  os << "//-------------------------------------------------------------------------" << endl;
  os << "/*! \\file host.cc" << endl << endl;
  os << "\\brief File generated by GeNN for the model " << model->name << " containing host control code." << endl;
  os << "*/" << endl;
  os << "//-------------------------------------------------------------------------" << endl << endl;
  os << endl;


  //---------------------
  // add host.cc includes

  os << "#include <cstdlib>" << endl;
  os << "#include <cmath>" << endl;
  os << "#include \"utils.h\"" << endl;
  os << "#ifndef RAND" << endl;
  os << "#define RAND(Y, X) Y = Y * 1103515245 + 12345; X = (unsigned int) (Y >> 16) & 32767" << endl;
  os << "#endif" << endl;
  os << "#include \"host.h\"" << endl;
  os << "#include \"neuron.cc\"" << endl;
  if (model->synapseGrpN > 0) {
    os << "#include \"synapse.cc\"" << endl;
  }
  os << endl;


  //-----------------------------
  // define host neuron variables

  os << "// neuron variable definitions" << endl;
  for (int i = 0; i < model->neuronGrpN; i++) {
    type = model->neuronType[i];
    os << "unsigned int glbscnt" << model->neuronName[i] << ";" << endl;
    os << "unsigned int *glbSpk" << model->neuronName[i] << ";" << endl;
    if (model->neuronDelaySlots[i] == 1) {
      os << "unsigned int glbSpkEvntCnt" << model->neuronName[i] << ";" << endl;
    }
    else {
      os << "unsigned int spkEvntQuePtr" << model->neuronName[i] << ";" << endl;
      os << "unsigned int *glbSpkEvntCnt" << model->neuronName[i] << ";" << endl;
    }
    os << "unsigned int *glbSpkEvnt" << model->neuronName[i] << ";" << endl;
    for (int j = 0; j < model->inSyn[i].size(); j++) {
      os << model->ftype << " *inSyn" << model->neuronName[i] << j << ";" << endl;
    } 
    for (int j = 0; j < nModels[type].varNames.size(); j++) {
      os << nModels[type].varTypes[j] << " *";
      os << nModels[type].varNames[j] << model->neuronName[i] << ";" << endl;
    }
    if (model->neuronNeedSt[i]) {
      os << model->ftype << " *sT" << model->neuronName[i] << ";" << endl;
    }
  }
  os << endl;


  //-----------------------------
  // define host synase variables

  os << "// synapse variable definitions" << endl;
  for (int i = 0; i < model->postSynapseType.size(); i++) {
    type = model->postSynapseType[i];
    for (int j = 0; j < postSynModels[type].varNames.size(); j++) {
      os << postSynModels[type].varTypes[j] << " *";
      os << postSynModels[type].varNames[j] << model->synapseName[i] << ";" << endl;
    }
  }
  for (int i = 0; i < model->synapseGrpN; i++) {
    if (model->synapseConnType[i] == SPARSE) {
      os << "Conductance g" << model->synapseName[i] << ";" << endl;
      //  os << "  " << model->ftype << " *gp" << model->synapseName[i] << ";" << endl;
      //  os << "  unsigned int *gp" << model->synapseName[i]  << endl;
      //  os << "  unsigned int *gp" << model->synapseName[i] << "ind;" << endl;
      //  os << "} ;" << endl;
    }
    else {
      if (model->synapseGType[i] == INDIVIDUALG) {
	os << model->ftype << " *gp" << model->synapseName[i] << ";" << endl;
      }
      if (model->synapseGType[i] == INDIVIDUALID) {
      	os << "unsigned int *gp" << model->synapseName[i] << ";" << endl;
      }	 
    }
    if (model->synapseType[i] == LEARN1SYNAPSE) {
      os << model->ftype << " *grawp" << model->synapseName[i] << ";" << endl;
    }
  }
  os << endl;
  os << "// host function definitions" << endl << endl;


  //----------------------------------------------------
  // host variable allocation code and memory estimation
  
  osh << "void allocateMemHost();" << endl;
  os << "void allocateMemHost()" << endl;
  os << "{" << endl;

  os << "  // neuron variables" << endl;
  for (int i = 0; i < model->neuronGrpN; i++) {
    type = model->neuronType[i];

    // allocate host neuron variables
    os << "  glbSpk" << model->neuronName[i] << " = new unsigned int[" << model->neuronN[i] << "];" << endl;
    if (model->neuronDelaySlots[i] == 1) {
      os << "  glbSpkEvnt" << model->neuronName[i] << " = new unsigned int[" << model->neuronN[i] << "];" << endl;
    }
    else {
      os << "  glbSpkEvntCnt" << model->neuronName[i] << " = new unsigned int[" << model->neuronDelaySlots[i] << "];" << endl;
      os << "  glbSpkEvnt" << model->neuronName[i] << " = new unsigned int[" << model->neuronN[i] * model->neuronDelaySlots[i] << "];" << endl;
    }
    for (int j = 0; j < model->inSyn[i].size(); j++) {
      os << "  inSyn" << model->neuronName[i] << j << " = new " << model->ftype << "[";
      os << model->neuronN[i] << "];" << endl;
    } 
    for (int j = 0; j < nModels[type].varNames.size(); j++) {
      os << "  " << nModels[type].varNames[j] << model->neuronName[i] << " = new " << nModels[type].varTypes[j] << "[";
      if ((nModels[type].varNames[j] == "V") && (model->neuronDelaySlots[i] != 1)) {
	os << (model->neuronDelaySlots[i] * model->neuronN[i]);
      }
      else {
	os << (model->neuronN[i]);
      }
      os << "];" << endl;
    }
    if (model->neuronNeedSt[i]) {
      os << "  sT" << model->neuronName[i] << " = new " << model->ftype << "[" << model->neuronN[i] << "];" << endl;
    }   
    os << endl; 
  }

  // allocate host synapse variables
  os << "  // synapse variables" << endl;
  for (int i = 0; i < model->synapseGrpN; i++) {
    if (model->synapseGType[i] == INDIVIDUALG) {
      // if (model->synapseConnType[i] == SPARSE) {
      /*********************If sparse, the arrays will be allocated later. ****************/
      //mem += model->neuronN[model->synapseSource[i]] * model->neuronN[model->synapseTarget[i]] * theSize(model->ftype); //TODO: This is actually less for sparse matrices but we need to find a way      
      //mem += model->neuronN[model->synapseSource[i]] * model->neuronN[model->synapseTarget[i]] * sizeof(int);
      //}
      //else {
      if (model->synapseConnType[i] != SPARSE) { 
	os << "  gp" << model->synapseName[i] << " = new " << model->ftype << "[";
	os << model->neuronN[model->synapseSource[i]] << " * " << model->neuronN[model->synapseTarget[i]];
      	os << "]; // synaptic conductances of group " << model->synapseName[i] << endl;
      }
      if (model->synapseType[i] == LEARN1SYNAPSE) {
	os << "  grawp" << model->synapseName[i] << " = new " << model->ftype << "[";
	os << model->neuronN[model->synapseSource[i]] << " * " << model->neuronN[model->synapseTarget[i]];
	os << "]; // raw synaptic conductances of group " << model->synapseName[i] << endl;
      }
    }
    // note, if GLOBALG we put the value at compile time
    if (model->synapseGType[i] == INDIVIDUALID) {
      os << "  gp" << model->synapseName[i] << " = new unsigned int[";
      tmp = model->neuronN[model->synapseSource[i]] * model->neuronN[model->synapseTarget[i]];
      size = tmp >> logUIntSz;
      if (tmp > (size << logUIntSz)) size++;
      os << size << "]; // synaptic connectivity of group " << model->synapseName[i] << endl;
    }
  }  
  for (int i = 0; i < model->postSynapseType.size(); i++) {
    type = model->postSynapseType[i];
    for (int j = 0; j < postSynModels[type].varNames.size(); j++) {
      os << "  " << postSynModels[type].varNames[j] << model->synapseName[i] << " = new " << postSynModels[type].varTypes[j] << "[" << (model->neuronN[model->synapseTarget[i]]) <<  "];" << endl;
    }
  }
  os << "}" << endl;
  os << endl;

  // memory usage estimation needs to be re-added!!!
  /*
  mos << "Global memory required for core model: " << mem/1e6 << " MB for all-to-all connectivity" << endl;
  mos << deviceProp[deviceID].totalGlobalMem << " this device: " << deviceID << endl;  
  if (0.5 * deviceProp[deviceID].totalGlobalMem < mem) {
    mos << "memory required for core model (" << mem/1e6;
    mos << "MB) is more than 50% of global memory on the chosen device";
    mos << "(" << deviceProp[deviceID].totalGlobalMem/1e6 << "MB)." << endl;
    mos << "Experience shows that this is UNLIKELY TO WORK ... " << endl;
  }
  */


  //--------------------------------------------------
  // allocating conductance arrays for sparse matrices

  osh << "void allocateSparseArrayHost(Conductance *C, unsigned int preN, bool isGlobalG);" << endl;
  os << "void allocateSparseArrayHost(Conductance *C, unsigned int preN, bool isGlobalG)" << endl;
  os << "{" << endl;

  os << "  if (isGlobalG == false) C->gp = new " << model->ftype << "[C->connN];" << endl; // synaptic conductances of group " << model->synapseName[i];
  //mem += gsize * theSize(model->ftype); //TODO: But we need to find a way
  os << "  C->gIndInG = new unsigned int[preN + 1];" << endl; // model->neuronN[model->synapseSource[i]] index where the next neuron starts in the synaptic conductances of group " << model->synapseName[i];
  os << "  C->gInd = new unsigned int[C->connN];" << endl; // postsynaptic neuron index in the synaptic conductances of group " << model->synapseName[i];
  //mem += gsize*sizeof(int);
  os << "}" << endl; 
  os << endl;


  //--------------------------------------------------
  // allocating conductance arrays for sparse matrices

  osh << "void allocateAllSparseArraysHost();" << endl;
  os << "void allocateAllSparseArraysHost()" << endl;
  os << "{" << endl;

  for (int i = 0; i < model->synapseGrpN; i++) {
    if (model->synapseConnType[i] == SPARSE) {
      os << "  allocateSparseArray(&g" << model->synapseName[i] << ", ";
      os << model->neuronN[model->synapseSource[i]] << ",";
      if (model->synapseGType[i] == GLOBALG) {
	os << " true); // globalG" << endl; 
      }
      else {
	os << " false);	// individual G" << endl;				
      }
    }
  }
  os << "}" << endl;
  os << endl;


  //--------------------------
  // initialize host variables

  osh << "void initializeHost();" << endl;
  os << "void initializeHost()" << endl;
  os << "{" << endl;

  // neuron variables
  //os << "  srand((unsigned int) time(NULL));" << endl;
  //os << "srand(101);" << endl;
  os << endl;
  os << "  // neuron variables" << endl;
  for (int i = 0; i < model->neuronGrpN; i++) {
    type = model->neuronType[i];
    os << "  glbscnt" << model->neuronName[i] << " = 0;" << endl;
    if (model->neuronDelaySlots[i] == 1) {
      os << "  glbSpkEvntCnt" << model->neuronName[i] << " = 0;" << endl;
    }
    else {
      os << "  spkEvntQuePtr" << model->neuronName[i] << " = 0;" << endl;
      os << "  for (int i = 0; i < " << model->neuronDelaySlots[i] << "; i++) {" << endl;
      os << "    glbSpkEvntCnt" << model->neuronName[i] << "[i] = 0;" << endl;
      os << "  }" << endl;
    }
    os << "  for (int i = 0; i < " << model->neuronN[i] << "; i++) {" << endl;
    os << "    glbSpk" << model->neuronName[i] << "[i] = 0;" << endl;
    if (model->neuronDelaySlots[i] == 1) {
      os << "    glbSpkEvnt" << model->neuronName[i] << "[i] = 0;" << endl;
    }
    else {
      os << "    for (int j = 0; j < " << model->neuronDelaySlots[i] << "; j++) {" << endl;
      os << "      glbSpkEvnt" << model->neuronName[i] << "[(j * " << model->neuronN[i] <<  ") + i] = 0;" << endl;
      os << "    }" << endl;
    }
    for (int j = 0; j < model->inSyn[i].size(); j++) {
      os << "    inSyn" << model->neuronName[i] << j << "[i] = 0.0f;" << endl;
    } 
    for (int j = 0; j < nModels[type].varNames.size(); j++) {
      if ((nModels[type].varNames[j] == "V") && (model->neuronDelaySlots[i] != 1)) {
	os << "    for (int j = 0; j < " << model->neuronDelaySlots[i] << "; j++) {" << endl;
	os << "      " << nModels[type].varNames[j] << model->neuronName[i] << "[(j * ";
	os << model->neuronN[i] << ") + i] = " << model->neuronIni[i][j] << ";" << endl;
	os << "    }" << endl;
      }
      else {
	os << "    " << nModels[type].varNames[j] << model->neuronName[i];
	os << "[i] = " << model->neuronIni[i][j] << ";" << endl;
      }
    }
    if (model->neuronType[i] == POISSONNEURON) {
      os << "    seed" << model->neuronName[i] << "[i] = rand();" << endl;
    } 
    if (model->neuronNeedSt[i]) {
      os << "    sT" <<  model->neuronName[i] << "[i] = -10000.0;" << endl;
    }
    os << "  }" << endl;
    if ((model->neuronType[i] == IZHIKEVICH) && (model->dt != 1)) {
      os << "  fprintf(stderr,\"WARNING: You use a time step different than 1 ms. Izhikevich model behaviour may not be robust.\\n\"); "<< endl;
    }
  }
  os << endl;

  // synapse variables
  os << "  // postsynapse variables" << endl;
  for (int i = 0; i < model->postSynapseType.size(); i++){
    type = model->postSynapseType[i];
    for (int j = 0; j < postSynModels[type].varNames.size(); j++) {
      os << "  for (int i = 0; i < " << model->neuronN[model->synapseTarget[i]] << "; i++) {" << endl;
      os << "    " << postSynModels[type].varNames[j] << model->synapseName[i];
      os << "[i] = " << model->postSynIni[i][j] << ";" << endl;
      os << "  }" << endl;
    }
  }
  os << "}" << endl;
  os << endl;


  //---------------------------------------
  // free dynamically allocated host memory

  osh << "void freeMemHost();" << endl;
  os << "void freeMemHost()" << endl;
  os << "{" << endl;

  for (int i = 0; i < model->neuronGrpN; i++) {
    type = model->neuronType[i];
    os << "  delete[] glbSpk" << model->neuronName[i] << ";" << endl;
    if (model->neuronDelaySlots[i] == 1) {
      os << "  delete[] glbSpkEvnt" << model->neuronName[i] << ";" << endl;
    }
    else {
      os << "  delete[] glbSpkEvntCnt" << model->neuronName[i] << ";" << endl;
      os << "  delete[] glbSpkEvnt" << model->neuronName[i] << ";" << endl;
    }
    for (int j = 0; j < model->inSyn[i].size(); j++) {
      os << "  delete[] inSyn" << model->neuronName[i] << j << ";" << endl;
    } 
    for (int j = 0; j < nModels[type].varNames.size(); j++) {
      os << "  delete[] " << nModels[type].varNames[j] << model->neuronName[i] << ";" << endl;
    }
    if (model->neuronNeedSt[i]) {
      os << "  delete[] sT" << model->neuronName[i] << ";" << endl;
    }
  }
  for (int i = 0; i < model->synapseGrpN; i++) {
    if (model->synapseConnType[i] == SPARSE){
      if (model->synapseGType[i] != GLOBALG) {
	os << "  delete[] g" << model->synapseName[i] << ".gp;" << endl;
      }
      os << "  delete[] g" << model->synapseName[i] << ".gIndInG;" << endl;
      os << "  delete[] g" << model->synapseName[i] << ".gInd;" << endl;  
    }
    else if (model->synapseGType[i] != GLOBALG) {
      os << "  delete[] gp" << model->synapseName[i] << ";" << endl;
    }
  }
  for (int i = 0; i < model->postSynapseType.size(); i++) {
    type = model->postSynapseType[i];
    for (int j = 0; j < postSynModels[type].varNames.size(); j++) {
      os << "  delete[] " << postSynModels[type].varNames[j] << model->synapseName[i] << ";" << endl;
    }
  }
  os << "}" << endl;
  os << endl;


  //--------------------------
  // learning helper functions

  if (model->lrnGroups > 0) {
    for (int i = 0; i < model->synapseGrpN; i++) {
      if (model->synapseType[i] == LEARN1SYNAPSE) {

	osh << model->ftype << " gFunc" << model->synapseName[i] << "Host(" << model->ftype << " graw);" << endl;
	os << model->ftype << " gFunc" << model->synapseName[i] << "Host(" << model->ftype << " graw)" << endl;
	os << "{" << endl;
	os << "  return " << SAVEP(model->synapsePara[i][8]/2.0) << " * (tanh(";
	os << SAVEP(model->synapsePara[i][10]) << " * (graw - ";
	os << SAVEP(model->synapsePara[i][9]) << ")) + 1.0);" << endl;
	os << "}" << endl;
	os << endl;

	osh << model->ftype << " invGFunc" << model->synapseName[i] << "Host(" << model->ftype << " g);" << endl;
	os << model->ftype << " invGFunc" << model->synapseName[i] << "Host(" << model->ftype << " g)" << endl;
	os << "{" << endl;
	os << model->ftype << " tmp = g / " << SAVEP(model->synapsePara[i][8]*2.0) << " - 1.0;" << endl;
	os << "return 0.5 * log((1.0 + tmp) / (1.0 - tmp)) /" << SAVEP(model->synapsePara[i][10]);
	os << " + " << SAVEP(model->synapsePara[i][9]) << ";" << endl;
	os << "}" << endl;
	os << endl;
      }
    }

    osh << "void initGRawHost();" << endl;
    os << "void initGRawHost()" << endl;
    os << "{" << endl;
    for (int i = 0; i < model->synapseGrpN; i++) {
      if (model->synapseType[i] == LEARN1SYNAPSE) {
	os << "  for (int i = 0; i < ";
	os << model->neuronN[model->synapseSource[i]] * model->neuronN[model->synapseTarget[i]] << "; i++) {" << endl;
	os << "    grawp"  << model->synapseName[i] << "[i] = invGFunc" << model->synapseName[i] << "Host";
	if (model->synapseGType[i] != GLOBALG) {
	  if (model->synapseConnType[i] == SPARSE) {
	    os << "(g" << model->synapseName[i] << ".gp[i]);" << endl;
	  }
	  else {
	    os << "(gp" << model->synapseName[i] << "[i]);" << endl;
	  }
	  os << "  }" << endl;
      	}
	else { // can be optimised: no need to create an array, a constant would be enough (TODO)
	  os << "(" << model->g0[i] << ");" << endl;
	}
      }
    }
    os << "}" << endl;
    os << endl;
  }


  // ----------------------------------
  // the actual time stepping procedure

  osh << "void stepTimeHost(";
  os << "void stepTimeHost(";
  for (int i = 0; i < model->neuronGrpN; i++) {
    if (model->neuronType[i] == POISSONNEURON) {
      osh << "unsigned int *rates" << model->neuronName[i] << ", ";
      os << "unsigned int *rates" << model->neuronName[i];
      os << ", // pointer to the rates of the Poisson neurons in grp " << model->neuronName[i] << endl;
      osh << "unsigned int offset" << model->neuronName[i] << ", ";
      os << "unsigned int offset" << model->neuronName[i];
      os << ", // offset on pointer to the rates in grp " << model->neuronName[i] << endl;
    }
    if (model->receivesInputCurrent[i] >= 2) {
      osh << "float *inputI" << model->neuronName[i] << ", ";
      os << "float *inputI" << model->neuronName[i];
      os << ", // pointer to the explicit input to neurons in grp " << model->neuronName[i] << ", " << endl;
    }
  }
  osh << model->ftype << " t);" << endl;
  os << model->ftype << " t)" << endl;
  os << "{" << endl;

  if (model->synapseGrpN>0) {
    os << "  if (t > 0.0) {" << endl; 
    os << "    calcSynapsesHost(t);" << endl;
    if (model->lrnGroups > 0) {
      os << "learnSynapsesPostHost(t);" << endl;
    }
    os << "  }" << endl;
  }
  os << "  calcNeuronsHost(";
  for (int i = 0; i < model->neuronGrpN; i++) {
    if (model->neuronType[i] == POISSONNEURON) {
      os << "rates" << model->neuronName[i] << ", ";
      os << "offset" << model->neuronName[i] << ", ";
    }
    if (model->receivesInputCurrent[i] >= 2) {
      os << "inputI" << model->neuronName[i] << ", ";
    }
  }
  os << "t);" << endl;
  os << "}" << endl;
  osh.close();
  os.close();

  //mos << "leaving genHostCode" << endl;
}
