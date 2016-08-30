

	postSynModel ps;



  // Add new postsynapse type - : 
  ps.varNames.clear();
  ps.varTypes.clear();
  ps.pNames.clear();
  
  ps.postSyntoCurrent = tS(" \
  0; \n \
  float in_NB = 0; \n \
     	in_NB += $(inSyn); \n \
");
  	 
	ps.postSynDecay = tS(" \
  	 	$(inSyn) = 0; \
  	");

  postSynModels.push_back(ps);
