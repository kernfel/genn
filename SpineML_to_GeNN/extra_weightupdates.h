
	weightUpdateModel wu;


  // Add new weightupdate type - : 
  wu.varNames.clear();
  wu.varTypes.clear();
  //wu.varNames.push_back(tS("g"));
  //wu.varTypes.push_back(tS("float"));
  wu.pNames.clear();
  
  wu.simCode = tS(" \
      { \n \
  	 $(addtoinSyn) = $(out_NB_pre); \n \
			$(updatelinsyn); \n \
			}");
  weightUpdateModels.push_back(wu);
