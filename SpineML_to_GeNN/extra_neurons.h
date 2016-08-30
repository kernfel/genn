

	neuronModel n;


			
  // Add new neuron type - passthroughNB: 
  n.varNames.clear();
  n.varTypes.clear();
  
  n.varNames.push_back(tS("V"));
  n.varTypes.push_back(tS("float"));
  // alias output ports
  
  n.varNames.push_back(tS("out_NB"));
  n.varTypes.push_back(tS("float"));
  n.varNames.push_back(tS("__regime_val"));
  n.varTypes.push_back(tS("int"));
  
  
  n.pNames.clear();
  
  n.dpNames.clear();
  n.resetCode = "";
  n.thresholdConditionCode = "";

  n.simCode = tS(" \
  	 float in_NB = 0; \n\
if (t > 400) { \n \
in_NB += 0.4; \n \
} \n \
else if (t > 200) { \n \
in_NB += 0.1; \n \
} \n \
else if (t > 0) { \n \
in_NB += 0.5; \n \
} \n \
 $(out_NB) = (in_NB); \n \
	 if ($(__regime_val)==1) { \n \
} \n \
oldSpike = false; \n \
  	");
  	
  	
			n.thresholdConditionCode = "true";
  	

  nModels.push_back(n);

		
			
  // Add new neuron type - LIF2: 
  n.varNames.clear();
  n.varTypes.clear();
  
  n.varNames.push_back(tS("V"));
  n.varTypes.push_back(tS("float"));
  n.varNames.push_back(tS("a_NB"));
  n.varTypes.push_back(tS("float"));
  n.varNames.push_back(tS("adap_NB"));
  n.varTypes.push_back(tS("float"));
  // alias output ports
  
  n.varNames.push_back(tS("out_NB"));
  n.varTypes.push_back(tS("float"));
  n.varNames.push_back(tS("__regime_val"));
  n.varTypes.push_back(tS("int"));
  
  
  n.pNames.clear();
  
  n.pNames.push_back(tS("tau_NB"));
  n.pNames.push_back(tS("m_NB"));
  n.pNames.push_back(tS("c_NB"));
  n.pNames.push_back(tS("tau_adap_NB"));
  n.dpNames.clear();
  n.resetCode = "";
  n.thresholdConditionCode = "";

  n.simCode = tS(" \
  	  $(out_NB) = (($(m_NB))*($(a_NB))+($(c_NB))); \n \
	 if ($(__regime_val)==1) { \n \
$(a_NB) += ((-($(a_NB))-($(adap_NB))+in_NB)/($(tau_NB)))*DT; \n \
	 	$(adap_NB) += ((in_NB-($(adap_NB)))/($(tau_adap_NB)))*DT; \n \
	 	} \n \
oldSpike = false; \n \
  	");
  	
  	
			n.thresholdConditionCode = "true";
  	

  nModels.push_back(n);

		