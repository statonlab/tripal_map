
// Classes:
//
// Marker data instantiated for Reference and Comparison chromosomes
// 

var configMapViewer = configMapViewer || {};

configMapViewer = {  

	MarkerData: class { 
		constructor(markerTypeDisplayStates, markerTypeColorMap) {
			this.linkageGroups = {}; // {linkageGroup: {mapName: {orientation: {lg}}}, linkageGroup2: {mapName2: {orientation: {lg}}}, ..}
			this.markerTypeDisplayStates = JSON.parse(markerTypeDisplayStates);
			this.markerTypeColorMap = markerTypeColorMap;
		}
		
		addLinkageGroup(mapMarkerData, linkageGroupName, linkageGroupMapName, linkageGroupId, mapId, orientation) {
			var lgroups = this.linkageGroups;
			var markerTypeDisplayStates = this.markerTypeDisplayStates;
			var markerTypeColorMap = this.markerTypeColorMap;

			mapMarkerData.forEach(function(markerData) {
				if (linkageGroupName != markerData.linkage_group) {
					return;	
				}

				// Obtain the marker display state
				var markerDisplayState = "Show";
				if (markerData.genetic_marker_type in markerTypeDisplayStates) {
					markerDisplayState = markerTypeDisplayStates[markerData.genetic_marker_type];
				} 

				// get marker color
				var markerColor = "";
				var markerType = markerData.genetic_marker_type;
				if (markerType in markerTypeColorMap) {
					markerColor = markerTypeColorMap[markerType]; // marker type exists in the colormap, assign the corresponding color
				}
				else {
					markerColor = "black"; // as a default set the marker color to black
				}
				
				var linkageGroup = "";
				var linkageGroupMapId = "";
				if (!(linkageGroupName in lgroups)) {
					// no linkage group of that name exists already
					lgroups[linkageGroupName] = {};
					lgroups[linkageGroupName][linkageGroupMapName] = {};
					linkageGroup = new configMapViewer.LinkageGroup(linkageGroupName, linkageGroupMapName, linkageGroupId, mapId, orientation);
					lgroups[linkageGroupName][linkageGroupMapName][orientation] = linkageGroup;
					lgroups[linkageGroupName][linkageGroupMapName][orientation].addMarker(markerData, linkageGroupMapName, markerDisplayState, markerColor);
				}
				else if ((linkageGroupName in lgroups) && 
				    (!(linkageGroupMapName in lgroups[linkageGroupName]))) { 
					lgroups[linkageGroupName][linkageGroupMapName] = {};
					// a linkage group of that name exists but it is from a different map
					linkageGroup = new configMapViewer.LinkageGroup( linkageGroupName, linkageGroupMapName, linkageGroupId, mapId, orientation);
					lgroups[linkageGroupName][linkageGroupMapName][orientation] = linkageGroup;
					lgroups[linkageGroupName][linkageGroupMapName][orientation].addMarker(markerData, linkageGroupMapName, markerDisplayState, markerColor);
				}
				else if ((linkageGroupName in lgroups) && 
					((linkageGroupMapName in lgroups[linkageGroupName])) && 
				    (!(orientation in lgroups[linkageGroupName][linkageGroupMapName]))) {
					// a linkage group of that name exists from the same map but a different orientation
					linkageGroup = new configMapViewer.LinkageGroup( linkageGroupName, linkageGroupMapName, linkageGroupId, mapId, orientation);
					lgroups[linkageGroupName][linkageGroupMapName][orientation] = linkageGroup;
					lgroups[linkageGroupName][linkageGroupMapName][orientation].addMarker(markerData, linkageGroupMapName, markerDisplayState, markerColor);
				}
				else {
					// the linkage group already exists. Add the new marker
					lgroups[linkageGroupName][linkageGroupMapName][orientation].addMarker(markerData, linkageGroupMapName, markerDisplayState, markerColor);
				}
			});

			this.organizeQTLs(linkageGroupName, linkageGroupMapName, orientation);
		}

		organizeQTLs(linkageGroupName, linkageGroupMapName, orientation) {
			// Now that all qtl markers are added to linkageGroup QTLs, place QTLs into QTLLanes object. 
			// It contains qtl marker references to qtls, with qtls organized in lanes. 
			// Seek optimal packing, and if multiple qtls with the same start and end position exist, 
			// superimpose them in the same lane.
			var linkageGroup = this.getLinkageGroup(linkageGroupName, linkageGroupMapName, orientation);
				
			// sort the QTLs by markerMinPos, then by markerMaxPos
			var QTLs = linkageGroup.QTLs; //{markerMinPos: {markerMaxPos: [marker1, marker2, ..], markerMaxPos2: [marker1, marker2,..]}, ..}
			var keyNumericSort = Object.keys(QTLs).map(function(a) {return parseFloat(a);}).sort(function (a, b) { return a-b; });
			var sortedQTLs = keyNumericSort.reduce((r, k) => (r[k] = QTLs[k], r), {});
			var ar = [];
			var tmaxPos = 0;
			Object.keys(sortedQTLs).forEach(function(minPos) {
				Object.keys(sortedQTLs[minPos]).forEach(function(maxPos) {
					sortedQTLs[minPos][maxPos].forEach(function(marker) {
					 ar.push({'minPos': minPos, 'maxPos': maxPos, 'feature_id': marker.feature_id});
					 if (parseFloat(maxPos) > parseFloat(tmaxPos)) {
						 tmaxPos = maxPos;
					 }
					});
				});
			});
			// place qtls in the QTLLanes data structure
			// [lane1: {markerMaxPos: [marker1, marker2 (all with same min/maxPos)], markerMaxPos2: [marker3]}, lane2: ..}
			var qlanes = linkageGroup.QTLLanes;
			var _this = this;
			Object.keys(sortedQTLs).forEach(function(markerMinPos) {
				Object.keys(sortedQTLs[markerMinPos]).forEach(function(markerMaxPos) {
					// place each QTL into a lane, start from the first. Create a new lane if does not fit into existing lanes
					var markerArray = sortedQTLs[markerMinPos][markerMaxPos];
					if (qlanes.length > 0) {
						var markerAddedToLane = false;
						var laneCount = 1;
						qlanes.forEach(function(lane) {
							var laneLength = lane.length;
							var lastMarkersObjMaxPos = Object.keys(lane[laneLength - 1])[0];
							if ((parseFloat(markerMinPos) >= parseFloat(lastMarkersObjMaxPos)) && (!markerAddedToLane))  {
								var obj = {};
								obj[markerMaxPos] = markerArray;
								lane.push(obj);
								markerAddedToLane = true;
							}
							laneCount += 1;
						});
						if (!markerAddedToLane) { // create new lane and add the markerArray
							_this.addMarkerToNewLane(markerMaxPos, markerArray, qlanes);
						}
					} 
					else {
						// no lanes exist, create a lane and add the markerArray
						_this.addMarkerToNewLane(markerMaxPos, markerArray, qlanes);
					}
				});
			});

			// using the QTLLanes data struct, create a mapping from feature id to lane (mapFeatureIdToLane) position for all QTLs.
			// Used during the node creation process, to assign the correct x (lane position) for QTL markers 
			var qlanes = linkageGroup.QTLLanes;
			var laneCount = 1;
			var laneMap = linkageGroup.mapFeatureIdToLane; 	// { featureId1: lanePos, featureId2: lanePos,..}
			qlanes.forEach(function(lane) {
				lane.forEach(function(markersObj) {
					var markers = Object.values(markersObj)[0]; //markersObj.maxPos;
					markers.forEach(function(marker) {
						// set the marker lane pos based on the feature_id
						laneMap[marker.feature_id] = laneCount;
					});
				});
				laneCount += 1;
			});
			
			// calculate the maxNum characters in the labels of each lane
			var qlanesLabels = linkageGroup.qlanesMaxLabelLen; // [ 3 /lane1/, 5 /lane2/, /lane3/ ..]
			laneCount = 1;
			qlanes.forEach(function(lane) {
				qlanesLabels.push(0);
				lane.forEach(function(markerArray) {
					var markers = Object.values(markerArray)[0]; 
					markers.forEach(function(marker) {
						var markerNameLen = marker.getDisplayName().length;
						if (qlanesLabels[laneCount-1] < markerNameLen) {
							qlanesLabels[laneCount-1] = markerNameLen;
						} 
					});
				});
				laneCount += 1;
			});
		}
		
		addMarkerToNewLane(markerMaxPos, markerArray, qlanes) {
			var obj = {};
			obj[markerMaxPos] = markerArray;
			var lane = [obj];
			qlanes.push(lane);
		}

		
		findCorrespondences() {
			// Acts on all linkage groups member data  
			// hash all markers according to name. keep only those with multiple occurrences (in multiple linkage groups)
			// hash format: 
			// {markername1: [{"mapName": mn1, "lgName":lg1, "position":pos1}, {"mapName": mn2, "lgName":lg2, "position":pos2},..],
			//  markername2: [{"mapName": mn1, "lgName":lg1, "position":pos1},..],..}
			var markerCorresHash = {};
			var _this = this;
			// iterate through all markers in all linkage groups and add each unique marker to the marker hash
	        Object.keys(this.linkageGroups).forEach( function(linkageGroupName) {
	        	Object.keys(_this.linkageGroups[linkageGroupName]).forEach( function(linkageGroupMapName) {
		        		var linkageGroup = _this.getLinkageGroup(linkageGroupName, linkageGroupMapName);
		        			Object.keys(linkageGroup.markers).forEach( function(markerName) {
		        				Object.keys(linkageGroup.markers[markerName]).forEach( function(markerPos) {
		        					var marker = linkageGroup.markers[markerName][markerPos];
		        					if (markerName in markerCorresHash) {
		        						markerCorresHash[markerName].push({"mapName": linkageGroupMapName, 
		        							"linkageGroupName": linkageGroupName, "position": markerPos });
		        					}
		        					else {
		        						markerCorresHash[markerName] = [];
		        						markerCorresHash[markerName].push({"mapName": linkageGroupMapName, 
		        							"linkageGroupName": linkageGroupName, "position": markerPos });
		        					}
		        				});
		        			});
		       		});
	        	});

	        var markerCorrespondences = {};
	        Object.keys(markerCorresHash).forEach( function(markerName) {
	        	if ((markerCorresHash[markerName].length) > 1) {
	        		markerCorrespondences[markerName] = markerCorresHash[markerName];
	        	}
	        });

	        Object.keys(this.linkageGroups).forEach( function(linkageGroupName) {
	        	Object.keys(_this.linkageGroups[linkageGroupName]).forEach( function(linkageGroupMapName) {
	        		var linkageGroup = _this.getLinkageGroup(linkageGroupName, linkageGroupMapName);
	        			linkageGroup.markerCorrespondences = markerCorrespondences;
	        	});
	        });

		}

		getMarkerColor(markerData) {
			var markerColor = "";
			var markerType = markerData.genetic_marker_type;
			if (markerType in this.markerTypeColorMap) {
				// if the marker type exists in the colormap, assign the corresponding color
				markerColor = this.markerTypeColorMap[markerType];
			}
			else {
				// as a default set the marker color to black
				markerColor = "black";
			}
			return markerColor;
		}

		setMarkerTypesWithColors() {
			var markerTypes = this.linkageGroups.getMarkerTypes();
			for( var markerType in this.markerTypeColorMap) {
				if (markerTypes.has(markerType)) {
					this.markerTypesWithColors[markerType] = this.markerTypeColorMap[markerType];
				}
			}
			return this.markerTypesWithColors;
		}

		getLinkageGroup(linkageGroupName, linkageGroupMapName, orientation=null ) {
			var ret = false;
			if ((linkageGroupName in this.linkageGroups) && 
				(linkageGroupMapName in this.linkageGroups[linkageGroupName])) {
				if (orientation == null) {
					Object.keys(this.linkageGroups[linkageGroupName][linkageGroupMapName]).forEach( function(linkageGroupOrientation) {
						orientation = linkageGroupOrientation;
					});
				}
				ret = this.linkageGroups[linkageGroupName][linkageGroupMapName][orientation];
			}
			return ret; 
		}

		getMarkerTypes(linkageGroupName=false, linkageGroupMapName=false) {
		
			var markerTypes = new Set();
			// get the marker types for a single specific linkage group
			if (linkageGroupName && linkageGroupMapName) {
				var linkageGroup = this.getLinkageGroup(linkageGroupName, linkageGroupMapName);
				markerTypes = linkageGroup.markerTypes;
			}
			else {
				// get the set of marker types for all linkage groups
				var lgroups = this.linkageGroups;
				var _this = this;
				Object.keys(this.linkageGroups).forEach(function(linkageGroupName) {
					Object.keys(lgroups[linkageGroupName]).forEach(function(linkageGroupMapName) {
						var linkageGroup = _this.getLinkageGroup(linkageGroupName, linkageGroupMapName);  
						markerTypes = new Set(function*() { yield* markerTypes; yield* linkageGroup.markerTypes; }());
					});
				});
			}
			return markerTypes;
		}
	},
	
	// data classes
	LinkageGroup: class {
		constructor(linkageGroupName, linkageGroupMapName, linkageGroupId, mapId, orientation) {
			this.name = linkageGroupName;
			this.mapName = linkageGroupMapName;
			this.id = linkageGroupId;
			this.mapId = mapId;
			this.orientation = orientation;
			this.markersMaxPos = 0;
			this.markerTypes = new Set();
			this.markers = {}; 		// {markerName: {markerPos: marker}, markerName2: {markerPos: marker},..}
			this.markerCorrespondences = {};
			this.QTLs = {}; 		// {markerMinPos: {markerMaxPos: [marker1, marker2, ..], markerMaxPos2: [marker1, marker2,..]}, ..}
			this.QTLLanes = [];  	// [{markerMaxPos: [marker1, marker2 (all with same min/maxPos)], markerMaxPos2: [marker3]}, lane2: ..]
			this.mapFeatureIdToLane = {}; // { featureId1: lanePos, featureId2: lanePos,..}
			this.qlanesMaxLabelLen = [];
			this.links = []; 		// pairs of nodes (marker labels)
			this.nodes = []; 		// marker labels
    		this.allNodes = [];
		}
		
		addMarker(markerData, linkageGroupMapName, markerDisplayState, markerColor) {

			var markerName = this.getMarkerName(markerData);
			var markerPos = this.getMarkerPosition(markerData);
			if (!(markerName in this.markers)) {	
				// create an array for the marker name - each marker name can be associated with multiple positions 
				this.markers[markerName] = {};
				var marker = this.createNewMarker(markerData, linkageGroupMapName, this.id, this.mapId, markerDisplayState, markerColor);
				this.markers[markerName][markerPos] = marker;
				if (markerData.genetic_marker_type == "QTL") {
					this.addQTL(marker, markerName, markerPos);
				}
				this.updateMarkersMaxPos(marker.getMaxPos()); // set new marker position maximum
				this.markerTypes.add(marker.type); // update marker types set in this linkage group, with this marker type
			} 
			else if ((markerName in this.markers) && (!(markerPos in this.markers[markerName]))) {
				var marker = this.createNewMarker(markerData, linkageGroupMapName, this.id, this.mapId, markerDisplayState, markerColor);
				this.markers[markerName][markerPos] = marker;
				if (markerData.genetic_marker_type == "QTL") {
					this.addQTL(marker, markerName, markerPos);
				}
				this.updateMarkersMaxPos(marker.getMaxPos()); // set new marker position maximum
				this.markerTypes.add(marker.type); // update marker types set in this linkage group, with this marker type
			}
		}

		addQTL(marker, markerName, markerPos) {
			// QTLs is an object which stores the QTLs according to start and end position
			// if start and end position are the same, put them in the same array. Else use a one element array
			// after all qtls are added to QTLs
			// this.QTLs = {markerMinPos: {markerMaxPos: [marker1, marker2, ..], markerMaxPos2: [marker1, marker2,..]}, ..}
			var markerMinPos = marker.getMinPos();
			var markerMaxPos = marker.getMaxPos();
			if (!(markerMinPos in this.QTLs)) {
				this.QTLs[markerMinPos] = {};
				if (!(markerMaxPos in this.QTLs[markerMinPos])) {
					this.QTLs[markerMinPos][markerMaxPos] = [];
					this.QTLs[markerMinPos][markerMaxPos].push(this.markers[markerName][markerPos]);
				}
				else {
					this.QTLs[markerMinPos][markerMaxPos].push(this.markers[markerName][markerPos]);
				}
			}
			else {
				if (!(markerMaxPos in this.QTLs[markerMinPos])) {
					this.QTLs[markerMinPos][markerMaxPos] = [];
					this.QTLs[markerMinPos][markerMaxPos].push(this.markers[markerName][markerPos]);
				}
				else {
					this.QTLs[markerMinPos][markerMaxPos].push(this.markers[markerName][markerPos]);
				}
			}
		}
	
		getMarkerTypes() {
			return this.markerTypes;
		}

		updateMarkersMaxPos(markerMaxPos) {
			if (parseFloat(markerMaxPos) > parseFloat(this.markersMaxPos)) {
				this.markersMaxPos = markerMaxPos;
			}
		}
		
		createNodeForAllMarkers(scaleRange, chrRectHeight) {

			var _this = this;
			Object.keys(this.markers).forEach(function(markerName) {
				Object.keys(_this.markers[markerName]).forEach( function(markerPos) {
					var marker = _this.markers[markerName][markerPos];
					if ((scaleRange(marker.pos) > -4 & scaleRange(marker.pos) < chrRectHeight + 4) && (marker.displayState == "Show")) {
						
						var QTL = (marker.type == 'QTL') ? true : false;
						var xPos = 0;
						var qtlLabelLen = 0;
						var height = 0;
						var heightDiff = (Math.abs(scaleRange(marker.getMaxPos())) - Math.abs(scaleRange(marker.getMinPos())));
						var QTLPeakHeight = 4;
						if (QTL) {
							xPos = _this.mapFeatureIdToLane[marker.feature_id];
							qtlLabelLen = _this.qlanesMaxLabelLen.slice(0,xPos-1).reduce((a, b) => a + b, 0);
							height = (heightDiff == 0 ) ? QTLPeakHeight : heightDiff;  
						}
				
						var node = { 
							x: xPos, // x must be the label offset (static) for the force model to work. In the qtl case, this is the lane number
							y: scaleRange(marker.pos), // y must be perturbed by the force model
							y_init: scaleRange(marker.pos),
							name: marker.getDisplayName(),
							fullName: marker.getFullName(),
							marker: marker,
							displayOnQTL: QTL,
							qtlMaxChrs: qtlLabelLen,
							height: height,
							correspondences: {},
						};
			    		_this.allNodes.push(node);
					}
				});
			});
		}
		
		
		setNodeCorrespondences(markerData) {
			// so after nodes are created and drawn, look up in the markerData.markerCorrespondences by the markername. 
	        // If the markerPos, lgname and mapname match the current node marker, 
			// obtain x1 (svg abs pos), y1 (from cur node), and draw line from there
	        // to the other linkage group, obtaining x2, (from svg abs pos) and y2 (from linkage group node)
		    var _this = this;
	
	    	var svg = d3.select("#select_fieldset_mapViewer_svg"); 
	        var svgrect = svg.node().getBoundingClientRect();
	        var svgScrollLeft = 0;
	        if (svg.select('#mv_scroll').node()) {
	        	svgScrollLeft = svg.select('#mv_scroll').node().scrollLeft;
	        }
	        
			this.nodes.forEach(function(node) {
				var marker = node.marker;
				
				if (node.fullName in _this.markerCorrespondences) {
			    	var corres = {"draw": false, "x1": 0, "x1_width":0,"y1": node.y, "x2": 0, "y2": 0, "x2_width":0}; 
			    	
			    	//obtain x1, y1
			    	var mnPrefix = "marker_names_";
			    	if (node.displayOnQTL) {
			    		mnPrefix = "QTL_marker_names_";
			    	}
			        var marker_names_ref_className_id = mnPrefix + marker.linkageGroupMapId + "_"+marker.linkageGroupId + "_"+mapViewer.RectangleEnum.ZOOM + "_" + tripalMap.encodeHtmlIdAttrib(node.fullName);
			        var marker_names_ref = svg.selectAll("#"+marker_names_ref_className_id);
			    
				    var marker_names_ref_rect = "";
				    if (marker_names_ref.node()) {
				    	marker_names_ref_rect = marker_names_ref.node().getBoundingClientRect();
					    corres["x1"] = marker_names_ref_rect.x - svgrect.x + svgScrollLeft;
					    corres["y1"] = marker_names_ref_rect.y - svgrect.y + marker_names_ref_rect.height/2;
					    corres["x1_width"] = marker_names_ref_rect.width;
				    }
				    
			    	// obtain x2, y2
			    	// search markerData for linkageGroup.name and linkageGroup.mapName
					// hash: {markername1: [{"mapName": mn1, "linkageGroupName":lg1, "position":pos1},{"mapName": mn2, "linkageGroupName":lg2, "position":pos2},..],
					var linkageGroupComp = markerData.getLinkageGroup(marker.linkageGroupName, marker.mapName);
					_this.markerCorrespondences[node.fullName].forEach(function(markerComp) {
						if (((markerComp.mapName != marker.mapName) || (markerComp.linkageGroupName != marker.linkageGroupName))) {
							linkageGroupComp = markerData.getLinkageGroup(markerComp.linkageGroupName, markerComp.mapName);
						} 
					});

					var marker_names_comp_className = mnPrefix + linkageGroupComp.mapId + "_"+linkageGroupComp.id + "_"+mapViewer.RectangleEnum.ZOOM + "_" + tripalMap.encodeHtmlIdAttrib(node.fullName);
					var marker_names_comp = svg.selectAll("#"+marker_names_comp_className);
					var marker_names_comp_rect = "";
					if (marker_names_comp.node()) {
						marker_names_comp_rect = marker_names_comp.node().getBoundingClientRect();
						corres["x2"] = marker_names_comp_rect.x - svgrect.x + svgScrollLeft;
						corres["y2"] = marker_names_comp_rect.y - svgrect.y + marker_names_comp_rect.height/2;
					    corres["x2_width"] = marker_names_comp_rect.width;
						corres["draw"] = true; // the node exists on the comparison linkage group. draw the correspondence
				
					}
					if (!((marker.linkageGroupMapId == linkageGroupComp.mapId) && (marker.linkageGroupId == linkageGroupComp.id))) {
						node["correspondences"] = corres;
					}
					else{
						node["correspondences"] = {"draw": false, "x1": 0, "x1_width":0,"y1": node.y, "x2": 0, "y2": 0, "x2_width":0};
					}
					
			    }
			});
		}

		createNodeAndLinkForAllMarkers(scaleRange, markerData, chrRectHeight) {

			this.links.length = 0;
			this.nodes.length = 0;

			var _this = this;
			Object.keys(this.markers).forEach(function(markerName) {
				Object.keys(_this.markers[markerName]).forEach( function(markerPos) {
					var marker = _this.markers[markerName][markerPos];
					if ((scaleRange(marker.pos) > -4 & scaleRange(marker.pos) < chrRectHeight + 4) && (marker.displayState == "Show")) {
						_this.setNodeAndLink(marker, scaleRange);
					}
				});
			});
		}

		setNodeAndLink(marker, scale) {
			// add all markers to links and nodes. On brush event check if the marker position fits 
			// within the rangeY of highlighted brush rect
			// for QTLs, use the map from feature id to lane to determine the x offset
			var QTL = (marker.type == 'QTL') ? true : false;
			var xPos = 0;
			var qtlLabelLen = 0;
			var height = 0;
			var heightDiff = (Math.abs(scale(marker.getMaxPos())) - Math.abs(scale(marker.getMinPos())));
			var QTLPeakHeight = 4;

			if (QTL) {
				xPos = this.mapFeatureIdToLane[marker.feature_id];
				// use reduce to sum our array
				qtlLabelLen = this.qlanesMaxLabelLen.slice(0,xPos-1).reduce((a, b) => a + b, 0);
				height = (heightDiff == 0 ) ? QTLPeakHeight : heightDiff;  

			}
			var node = { 
				x: xPos, // x must be the label offset (static) for the force model to work.
				y: scale(marker.pos), // y must be perturbed by the force model
				y_init: scale(marker.pos),
				name: marker.getDisplayName(),
				fullName: marker.getFullName(),
				marker: marker,
				displayOnQTL: QTL,
				qtlMaxChrs: qtlLabelLen,
				height: height,
				correspondences: {},
			};
    		this.nodes.push(node);
    		var numNodes = this.nodes.length;
    		if (numNodes > 1) {
    			// other nodes already exist, create a node pair (for marker labels in the force model)
    			this.links.push({source: this.nodes[numNodes-2], target: node});
    		}
		}
		
		getMarkerName(markerData) {
			
			var markerName = "";
			if (markerData.genetic_marker_type == "QTL") {
				markerName = markerData.genetic_marker_name;
			}
			else {
				markerName = markerData.genetic_marker_locus_name;
			}
			return markerName;
		}

		getMarkerPosition(markerData) {
			var markerPos = 0;
			if (markerData.genetic_marker_type == "QTL") {
				markerPos = (markerData.hasOwnProperty("marker_start_pos")) ? 
					markerData.marker_start_pos : (markerData.hasOwnProperty("marker_qtl_peak")) ? 
					markerData.marker_qtl_peak : (markerData.hasOwnProperty("marker_stop_pos")) ? 
					markerData.marker_stop_pos : markerPos; 
			}
			else {
				markerPos = (markerData.hasOwnProperty("marker_start_pos")) ? 
					markerData.marker_start_pos : markerPos;
			}
			return markerPos;
		}
		
		createNewMarker(markerData, linkageGroupMapName, linkageGroupId, linkageGroupMapId, markerDisplayState, markerColor) {
			var marker = "";
			if (markerData.genetic_marker_type == "QTL") {
				marker = new configMapViewer.QTLMarker(markerData, linkageGroupMapName, linkageGroupId, linkageGroupMapId, markerDisplayState, markerColor);
			}
			else if (markerData.genetic_marker_type == "heritable_phenotypic_marker") {
				marker = new configMapViewer.HeritablePhenotypicMarker(markerData, linkageGroupMapName, linkageGroupId, linkageGroupMapId, markerDisplayState, markerColor);
			}
			else {
				// genetic marker
				marker = new configMapViewer.GeneticMarker(markerData, linkageGroupMapName, linkageGroupId, linkageGroupMapId, markerDisplayState, markerColor);
			}
			return marker;
		}
		getMarkersMaxPos() {
			return this.markersMaxPos;
		}
	},
	 
	Marker: class {
		constructor(markerData, markerDisplayState, markerColor) {
			this.pos = 0;
			this.name = markerData.genetic_marker_name;
		}
		assignMarkerData(markerData) {
			return markerData;
		}
		getDisplayName() {
			return this.name;
		}
		getFullName() {
			return this.name;
		}
		getPos() {
			return this.pos;
		}
	},
		
	GeneticMarker: class /*extends Marker: class*/ {
		constructor(markerData, linkageGroupMapName, linkageGroupId, linkageGroupMapId, markerDisplayState, markerColor) {
			this.kind = 'genetic_marker';
			this.type = markerData.genetic_marker_type;
			this.name = markerData.genetic_marker_locus_name;
			this.linkageGroupName = markerData.linkage_group;
			this.mapName = linkageGroupMapName;
			this.linkageGroupId = linkageGroupId;
			this.linkageGroupMapId = linkageGroupMapId;
			this.pos = (markerData.hasOwnProperty("marker_start_pos")) ? 
					parseFloat(markerData.marker_start_pos) : 0;
			this.feature_id = markerData.feature_id;		
			this.displayState = markerDisplayState;
			this.color = markerColor;
			this.url = markerData.feature_url;
		}
		getDisplayName() {
			return this.name;
		}
		getFullName() {
			return this.name;
		}
		getMaxPos() {
			return this.pos;
		}
		getMinPos() {
			return this.pos;
		}
		
	},
	HeritablePhenotypicMarker: class /*extends Marker*/ {
		constructor(markerData, linkageGroupMapName, linkageGroupId, linkageGroupMapId, markerDisplayState, markerColor) {
			this.kind = "heritable_phenotypic_marker"; 
			this.type = markerData.genetic_marker_type;
			this.name = markerData.genetic_marker_locus_name;
			this.linkageGroupName = markerData.linkage_group;
			this.linkageGroupId = linkageGroupId;
			this.linkageGroupMapId = linkageGroupMapId;
			this.mapName = linkageGroupMapName;
			this.pos = (markerData.hasOwnProperty("marker_start_pos")) ? 
					parseFloat(markerData.marker_start_pos) : 0;
			this.feature_id = markerData.feature_id;		
			this.displayState = markerDisplayState;
			this.color = markerColor;
			this.url = markerData.feature_url;
		}
		// subclassed method which overrides their superclass method of the same name.
		assignMarkerData(markerData) {
			//super.assignMarkerData(markerData);
		}
		getDisplayName() {
			return this.name;
		}
		getFullName() {
			return this.name;
		}
		getMaxPos () {
			return this.pos;
		}
		getMinPos() {
			return this.pos;
		}
	
	},

	QTLMarker: class /*extends Marker*/ {
		constructor(markerData, linkageGroupMapName, linkageGroupId, linkageGroupMapId, markerDisplayState, markerColor) {
			this.kind = 'QTL';
			this.type = markerData.genetic_marker_type;
			this.name = markerData.genetic_marker_locus_name;
			this.nameAbbrev = markerData.marker_name_abbrev;
			this.linkageGroupName = markerData.linkage_group;
			this.linkageGroupId = linkageGroupId;
			this.linkageGroupMapId = linkageGroupMapId;
			this.mapName = linkageGroupMapName;
			this.startPos = (markerData.hasOwnProperty("marker_start_pos")) ? parseFloat(markerData.marker_start_pos) : false;
			this.QTLPeak = (markerData.hasOwnProperty("marker_qtl_peak")) ? parseFloat(markerData.marker_qtl_peak) : false;
			this.stopPos = (markerData.hasOwnProperty("marker_stop_pos")) ? parseFloat(markerData.marker_stop_pos) : false;
			this.feature_id = markerData.feature_id;		
			this.displayState = markerDisplayState;
			this.color = markerColor;
			this.maxPos = this.getMaxPos();
			this.minPos = this.getMinPos();
			this.pos = this.minPos;
			this.url = markerData.feature_url;
		}
		// subclassed method which overrides superclass method of the same name.
		assignMarkerData(markerData) {
		}
		getDisplayName() {
			return this.nameAbbrev;
		}
		getFullName() {
			return this.name;
		}
		getMaxPos() {
			var maxPos = (this.stopPos) ? this.stopPos : (this.QTLPeak) ?  this.QTLPeak : (this.startPos) ? this.startPos : 0;
			return maxPos;
		}
		getMinPos() {
			var minPos = (this.startPos) ? this.startPos : (this.QTLPeak) ?  this.QTLPeak : (this.stopPos) ? this.stopPos : 0;
			return minPos;
		}
	
	},
		
};
