
// Classes:
// ChrFrame (instantiated as Reference and Comparison chromosomes)
//   contains MapLabel, and instances of ChrView and ZoomedView classes
//
// ChrView
//
// ZoomedView
//
//
var mapViewer = mapViewer || {};

mapViewer = {  

   OrientationEnum: {LEFT: 0, RIGHT: 1},
   RectangleEnum: {CHR: 0, ZOOM: 1},
	
	ChrFrame: class{
		
		constructor(orientation, show, mapName, mapId, linkageGroupName, markerData) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.show = show;
			this.svg = 0;
			this.containerName = "chrFrame_"+orientation;
			this.orientation = orientation;
			this.chrRect = 	{height: 400, width: 25};
			this.linkageGroup = markerData.getLinkageGroup(linkageGroupName, mapName);
			this.markerData = markerData;
			this.markerEvents = new mapViewer.MarkerEvents(this.linkageGroup, markerData, show, this.chrRect);
			
			this.mapLabel = new mapViewer.MapLabel(orientation, mapName, mapId, linkageGroupName);
			this.chrView = new mapViewer.ChrView(orientation, show, this.linkageGroup, this.chrRect, this.markerEvents);
			this.zoomedView = new mapViewer.ZoomedView(orientation, show, this.linkageGroup, 
					markerData, this.chrRect, this.markerEvents);
			this.correspondences = new mapViewer.Correspondences(orientation, this.linkageGroup, markerData, this.markerEvents);

			// initialize / subscribe chrView and zoomedView to markerEvents so on brushScroll, they will be updated with new domain.
			this.markerEvents.chrView = this.chrView;
			this.markerEvents.zoomedView = this.zoomedView;
			this.markerEvents.correspondences = this.correspondences;

		}

		calculateWidth(svgParent) {
			svgParent.selectAll("#"+this.containerName).remove(); 
			// chromosome frame 
			this.drawChrFrame(svgParent);
			var tmpTranslateX = 0;
			var tmpTranslateY = 0;
			this.mapLabel.draw(this.svg);
			tmpTranslateY += this.mapLabel.height + 40;
			if (this.orientation == mapViewer.OrientationEnum.LEFT) {
				this.chrView.setTranslate(tmpTranslateX, tmpTranslateY);
				this.chrView.draw(this.svg);
				tmpTranslateX += this.chrView.width;
				this.zoomedView.setTranslate(tmpTranslateX, tmpTranslateY);				
				this.zoomedView.draw(this.svg);
			}
			else {
				this.zoomedView.setTranslate(tmpTranslateX, tmpTranslateY);
				this.zoomedView.draw(this.svg);

				var chrFrameRect = this.svg.node().getBoundingClientRect();
				var zoomedViewWidth = this.zoomedView.x() - chrFrameRect.x;
				tmpTranslateX += zoomedViewWidth;
				this.chrView.setTranslate(tmpTranslateX, tmpTranslateY);
				this.chrView.draw(this.svg);
			}

			var width = this.svg.node().getBBox().width;
			this.chrView.width = 0;
			this.zoomedView.width = 0;
		
			svgParent.selectAll("#"+this.containerName).remove(); 
			return width; 
		}
		
		draw(svgParent) {

			// chromosome frame 
			this.drawChrFrame(svgParent);
			var tmpTranslateX = 0;
			var tmpTranslateY = 0;
			
			this.mapLabel.setTranslate(tmpTranslateX, tmpTranslateY);
			this.mapLabel.draw(this.svg);
			tmpTranslateY += this.mapLabel.height + 40;

			if (this.orientation == mapViewer.OrientationEnum.LEFT) {
				this.chrView.setTranslate(tmpTranslateX, tmpTranslateY);

				this.chrView.draw(this.svg);
				tmpTranslateX += this.chrView.width;
				this.zoomedView.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.zoomedView.draw(this.svg);
				if (this.show.Comparison) {
					this.linkageGroup.setNodeCorrespondences(this.markerData);
				}
			}
			else {
				this.zoomedView.setTranslate(tmpTranslateX, tmpTranslateY);
				this.zoomedView.draw(this.svg);

				var chrFrameRect = this.svg.node().getBoundingClientRect();
				var zoomedViewWidth = this.zoomedView.x() - chrFrameRect.x;
				tmpTranslateX += zoomedViewWidth;
				this.chrView.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.chrView.draw(this.svg);
				
				if (this.show.Comparison) {
					this.linkageGroup.setNodeCorrespondences(this.markerData);
					this.correspondences.setTranslate(0, 0);
					this.correspondences.draw(this.svg);
				}
			}

			this.chrView.brush();
			
			this.markerEvents.startForce(this.markerEvents.forceZoomedView, this.zoomedView); // this must be done with each brush refresh on chr inside ZoomedView
	    	
			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}

		
		drawChrFrame(svgParent) {
			
			var chrFrame = svgParent.append("g")
			.attr("id", this.containerName)
			.attr("transform", "translate("+ this.translateX + "," + this.translateY + ")")
			.attr("visibility", "unhidden");
			this.svg = chrFrame;
			
			return this.svg;
		}
		
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}
		
	},
	
	MarkerEvents: class {
		constructor(linkageGroup, markerData, show, chrRect) {
			this.chrView = "";
			this.zoomedView = "";
			this.correspondences = "";
			this.chrRect = chrRect;
			this.show = show;
			this.linkageGroup = linkageGroup;
			this.markerData = markerData;
			this.markersMaxPos = linkageGroup.getMarkersMaxPos();
			this.initScaleRange = this.setInitialScaleRangeAndDomain(this.initScaleRange, .1);
			this.chrScaleRange = this.setInitialScaleRangeAndDomain(this.chrScaleRange, 1);
			this.zoomScaleRange = this.setInitialScaleRangeAndDomain(this.zoomScaleRange, 1);
			
			// set the force model - only the zoom view has force model and not chromosome view
			this.forceZoomedView = this.createForce(this.chrRect, this.linkageGroup); 

			// only create nodes for all markers once on initialization, this will be used for container width calculations
			this.linkageGroup.createNodeForAllMarkers(this.chrScaleRange, chrRect.height); 

			// create the dynamic list of nodes and links. This is updated on each brush scroll
			this.linkageGroup.createNodeAndLinkForAllMarkers(this.chrScaleRange, this.markerData, chrRect.height);

	        // the ChrView and Zoomed view will be set by chromosome frame class once initialized
	        this.chrView = "";
	        this.zoomedView = "";
		}
		
		setInitialScaleRangeAndDomain(scale, zoomedSubsection) {
			// Update the scale domain: domain is the dataset size, range is the svg dimension to render
			//use the marker data to determine the max y domain ( stop value is higher than the start_pos or qtl_peak)
			var scaleRange = d3.scaleLinear().range([0, this.chrRect.height]);
			// set initial zoomed domain to display only a small subsection of markers on launch
			scaleRange.domain([0, this.markersMaxPos*zoomedSubsection]);

			return scaleRange;
		}

		
		onBrushScroll(brushRectScale) {
		
			// then apply the domain from the brushRect scale
			this.chrScaleRange.domain(brushRectScale.domain());

			this.linkageGroup.createNodeAndLinkForAllMarkers(this.chrScaleRange, this.markerData, this.chrRect.height);
			
			// update the chrView and zoomedView
			this.chrView.update(this.chrView.svg);
			this.zoomedView.update(this.zoomedView.svg);
			if (this.show.Comparison) {
				this.linkageGroup.setNodeCorrespondences(this.markerData);
				this.correspondences.setTranslate(0, 0);
				this.correspondences.update(this.svg);
			}
			this.startForce(this.forceZoomedView, this.zoomedView);
		}
		
		getFilteredNodes(displayFilter, type = null, all = false) {

			var _nodes = this.linkageGroup.nodes;
			var _allNodes = this.linkageGroup.allNodes;
			var _nodesChrRect = [];
			var _nodesChrQTL = [];
		
			if (type == mapViewer.RectangleEnum.CHR) {
				// iterate through the rest of the markers in the lane, adding additional markers at vertical (y)
		    	// increments as a percentage of the vertical range.
				// sort the nodes lowest to highest start pos.
				var sortedNodes = _allNodes.sort(function (a, b) { return a.y_init - b.y_init; });
				
				// handle qtls and marker loci separately
				// first marker loci
				var allMarkerLociNodes = sortedNodes.filter(function(d) {return (!d.displayOnQTL);});
				var maxPos = d3.max(allMarkerLociNodes, function(node) { return node.marker.getMaxPos(); }); 
				var minPos = d3.min(allMarkerLociNodes, function(node) { return node.marker.getMinPos(); }); 
				var sectionSize = maxPos*.05; 
		        var arrayPos = 0;
		        var curPos = minPos;
		        while(arrayPos < allMarkerLociNodes.length) {
		        	if (allMarkerLociNodes[arrayPos].marker.getMinPos() >= curPos ) {
		        		_nodesChrRect.push(allMarkerLociNodes[arrayPos]);
		        		curPos = (allMarkerLociNodes[arrayPos].marker.getMinPos()) + sectionSize;
		        	}
		        	arrayPos += 1;
		        }
		        
				// then qtls
				var allQTLNodes = sortedNodes.filter(function(d) {return (d.displayOnQTL);});
				maxPos = d3.max(allQTLNodes, function(node) { return node.marker.getMaxPos(); }); 
				minPos = d3.min(allQTLNodes, function(node) { return node.marker.getMinPos(); }); 
				sectionSize = maxPos*.05; 
		        arrayPos = 0;
		        curPos = minPos;
		        while(arrayPos < allQTLNodes.length) {
		        	if (allQTLNodes[arrayPos].marker.getMinPos() >= curPos ) {
		        		_nodesChrQTL.push(allQTLNodes[arrayPos]);
		        		curPos = (allQTLNodes[arrayPos].marker.getMinPos()) + sectionSize;
		        	}
		        	arrayPos += 1;
		        }

			}

			var retNodes = [];

			if (displayFilter == "displayOnRect") {
				if (type == mapViewer.RectangleEnum.CHR) {
					retNodes = _nodesChrRect;
				}
				else { // zoom
					if (all) {
						retNodes = _allNodes.filter(function(d) {return (!d.displayOnQTL);});
					}
					else {
						retNodes = _nodes.filter(function(d) {return (!d.displayOnQTL);});
					}
				}
			}
			else { 
				// display filter is "displayOnQTL", reset the nodes for now
				if (type == mapViewer.RectangleEnum.CHR) {
					retNodes = _nodesChrQTL;//.filter(function(d) {return (d.displayOnQTL);});
				}
				else { // zoom
					if (all) {
						retNodes = _allNodes.filter(function(d) { return d.displayOnQTL;});
					}
					else {
						retNodes = _nodes.filter(function(d) { return d.displayOnQTL;});
					}
				}
			}
			
			if (displayFilter == "correspondences"){
				retNodes = _nodes.filter(function(d) {return (d.correspondences.draw)});
			}
			return retNodes;  			
		}
		
		mouseover(tip, node, linkageGroupName, mapName, svg) {

			svg.selectAll("text").selectAll("text#popup").remove();
			svg.classed("active", true);
			
			// change the cursor to a hand when hovering over
			svg.selectAll("line, text").style("cursor", "pointer");
			
			// text aggregate popup
			var pos = '';
			if ((node.marker.kind == "QTL") && (node.marker.startPos != node.marker.stopPos)) {
				pos = Drupal.t('Position: @startPos - @stopPos', {
					'@startPos': node.marker.startPos, '@stopPos': node.marker.stopPos}); 
			}
			else {
				pos = Drupal.t('Position: @pos', {'@pos': node.marker.pos}); 
			}

			linkageGroupName = tripalMap.lGNameReduction(linkageGroupName, mapName);
			
			tip.transition().duration(200).style("opacity", .9);      
			tip.html(Drupal.t('Map: @map', {'@map': mapName}) + "<br/>"  + 
					Drupal.t('Linkage Group: @group', {'@group': linkageGroupName}) + "<br/>" + 
					pos + "<br/>" +
					Drupal.t('Type: @markerType', {'@markerType': node.marker.type}) + "<br/>" + 
					Drupal.t('Name: @markerName', {'@markerName': node.marker.name})
					)  
				.style("left", (d3.event.pageX + 20) + "px")     
				.style("top", (d3.event.pageY - 28) + "px");
			
	    }
	    
		mouseout(tip, svg) {

			tip.transition().duration(500).style("opacity", 0);
			svg.classed("active", false)
	            .selectAll("line, text")
	            .style("cursor", "default");
		}

		onclick(node) {
			window.open(Drupal.settings.baseUrl+"/tripalmap_feature/"+node.marker.feature_id);		

			return false; 
		}

		startForce(force, view) {
			force.on("tick", function() {
				view.onForceTick(view.svg);
			});
			
			force.start();
	    	for (var i = 25; i > 0; --i) force.tick();
	    	force.stop();
	
	    	// expose and animate final iterations showing the labels further spread apart
	    	force.start();
	    	setTimeout(stopanimate, 1000);
	    	
	    	var _force = force;
	    	function stopanimate() { _force.stop(); }	    	
		}

		createForce(chrRect, linkageGroup) {
			var force = d3.layout.force()
        	.nodes(linkageGroup.nodes)
        	.links(linkageGroup.links)
			.gravity(0) // in the range [0,1], centers the nodes
			.friction(0.15) // range [0,1], scales the particle velocity (decay), 1: frictionless, 0: freezes all particles in place 
			.linkStrength(0) // rigidity of links in the range [0,1], seems to center the nodes
			.linkDistance(30)
			.charge(-60) // negative value gives node repulsion
			.size([1000, chrRect.height + 100]);
			
			return force;
		}
		getchrScaleRange() {
			return this.chrScaleRange;
		}
		getzoomScaleRange() {
			return this.zoomScaleRange;
		}

	},
	
	ChrView: class {
		
		constructor(orientation, show, linkageGroup, chrRect, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.showRuler = show.Ruler;
			this.linkageGroup = linkageGroup;
			this.chrRect = chrRect;
			this.markerEvents = markerEvents;
			this.scale = this.markerEvents.chrScaleRange; 
			this.ruler = new mapViewer.Ruler(orientation, mapViewer.RectangleEnum.CHR, this.markerEvents);
			this.chr = new mapViewer.Chr(orientation, linkageGroup, chrRect, this.markerEvents);
			this.polygon = new mapViewer.Polygon(orientation, this.markerEvents);
			this.qtl = new mapViewer.QTL(orientation, mapViewer.RectangleEnum.CHR, linkageGroup, this.markerEvents);
			
		}

		brush() {
			this.chr.brush.onBrushScroll();
		}
		
		draw(svgParent) {
			
			this.drawChrView(svgParent);
			var tmpTranslateX = 0;
			var tmpTranslateY = 0;			
			
			if (this.orientation == mapViewer.OrientationEnum.LEFT ) {

				if (this.showRuler) {
					tmpTranslateX = this.ruler.calculateWidth(this.svg);
					this.ruler.setTranslate(tmpTranslateX, tmpTranslateY);

					this.ruler.draw(this.svg);
				}	
				var qtlWidth = this.qtl.calculateWidth(this.svg);
				if (qtlWidth > 0) {
					tmpTranslateX += qtlWidth;
					this.qtl.setTranslate(tmpTranslateX, tmpTranslateY);
					
					this.qtl.draw(this.svg);
				}
				var chrWidth = this.chr.calculateWidth(this.svg);
				tmpTranslateX += chrWidth - this.chrRect.width + 10;
				this.chr.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.chr.draw(this.svg);
				tmpTranslateX += this.chrRect.width;
				this.polygon.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.polygon.draw(this.svg);
			}
			else {
				this.polygon.setTranslate(tmpTranslateX, tmpTranslateY) - 5;
				
				this.polygon.draw(this.svg);
				tmpTranslateX += this.polygon.width;
				this.chr.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.chr.draw(this.svg);
				tmpTranslateX += this.chr.width;
				this.qtl.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.qtl.draw(this.svg);
				tmpTranslateX += this.qtl.svg.node().getBBox().width + 5;

				if (this.showRuler) {
					this.ruler.setTranslate(tmpTranslateX, tmpTranslateY);
					this.ruler.draw(this.svg);
				}
			}

			this.height = this.svg.node().getBBox().height;
			this.width += this.svg.node().getBBox().width;

			return svgParent;

		}
		update(svgParent) {
			this.chr.update(svgParent); 
			this.polygon.update(svgParent);

			return svgParent;
		}
		
		onForceTick(svgParent) {
			this.chr.onForceTick(svgParent);
			this.qtl.onForceTick(svgParent);
			
		}
		
		drawChrView(svgParent) {
			var chrView = svgParent.append("g")
			.attr("id", "chrView").attr( "transform", "translate(" + this.translateX + "," + this.translateY + ")")
			.attr("visibility", "unhidden");
		
			this.svg = chrView;
			return this.svg
		}
		
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},
	
	ZoomedView: class {
			
		constructor(orientation, show, linkageGroup, markerData, chrRect, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.linkageGroup = linkageGroup;
			this.chrRect = chrRect;
			this.showRuler = show.Ruler;
			this.showMarkerPos = show.MarkerPos;
			this.markerEvents = markerEvents;
			this.ruler = new mapViewer.Ruler(orientation, mapViewer.RectangleEnum.ZOOM, this.markerEvents);
			this.zoom = new mapViewer.Zoom(orientation, this.showMarkerPos, linkageGroup, chrRect, this.markerEvents);

			var QTLOrientation = mapViewer.OrientationEnum.LEFT;
			if (this.orientation == mapViewer.OrientationEnum.LEFT) {
				// for the zooomed view, the marker labels face the opposite direction to the chr labels
				QTLOrientation = mapViewer.OrientationEnum.RIGHT;
			}				
			this.qtl = new mapViewer.QTL(QTLOrientation, mapViewer.RectangleEnum.ZOOM, linkageGroup, this.markerEvents);

		}

		x() {
			
			var x = 0;
			if (this.orientation != mapViewer.OrientationEnum.LEFT) {
				if (this.zoom.svg.selectAll("rect").node()) {
					x = this.zoom.svg.selectAll("rect").node().getBoundingClientRect().right;
				}
			}
			return x;
		}
		
		draw(svgParent) {
		
			this.drawZoomedView(svgParent);
			var tmpTranslateX = 0;
			var tmpTranslateY = 0;
			
			if (this.orientation == mapViewer.OrientationEnum.LEFT ) {

				if (this.showRuler && !this.showMarkerPos) {
					// create ruler on zoomed view if not showing each individual marker position
					this.ruler.setTranslate(tmpTranslateX, tmpTranslateY);
					
					this.ruler.draw(this.svg);
				}				
				this.zoom.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.zoom.draw(this.svg);
				tmpTranslateX = this.zoom.width+10;
				this.qtl.setTranslate(tmpTranslateX, tmpTranslateY);

				this.qtl.draw(this.svg);
				this.width = this.svg.node().getBBox().width;
				
			}
			else {
				
				var qtlWidth =  this.qtl.calculateWidth(this.svg);
				if (qtlWidth > 0) {
					tmpTranslateX = qtlWidth;
					this.qtl.setTranslate(tmpTranslateX, tmpTranslateY);

					this.qtl.draw(this.svg);
				}
				tmpTranslateX += 5;
				this.zoom.setTranslate(tmpTranslateX, tmpTranslateY);
				
				this.zoom.draw(this.svg);
				tmpTranslateX += this.zoom.width + 5;
				this.ruler.setTranslate(tmpTranslateX, tmpTranslateY);

				if (this.showRuler) {
					if (!this.showMarkerPos) {
						this.ruler.draw(this.svg);
					}
				}
	
			}
			
			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;
			return svgParent;

		}

		update(svgParent) {
			if (this.showRuler && !this.showMarkerPos) {
				this.ruler.update(svgParent);
			}
			this.qtl.update(svgParent);
			this.zoom.update(svgParent);
			
			return svgParent;
		}
	
		onForceTick(svgParent) {
			this.zoom.onForceTick(svgParent);
			this.qtl.onForceTick(svgParent);
		}

		drawZoomedView(svgParent) {

	        var zoomedView = svgParent.append("g")
    		.attr("id", "zoomedView").attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
    		.attr("visibility", "unhidden");
			
			this.svg = zoomedView;
			
			return this.svg;
		}

		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}
	},

	
	Polygon: class {
		constructor(orientation, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.markerEvents = markerEvents;
			this.scale = this.markerEvents.chrScaleRange;
			this.markersMaxPos = this.markerEvents.markersMaxPos;
			this.chrRect = markerEvents.chrRect;
			this.chrHeight = this.chrRect.height;
			this.chrWidth = this.chrRect.width;
			this.constant = this.chrHeight/this.markersMaxPos;

		}
		draw(svgParent) {

			var minScaleDomain = d3.min(this.scale.domain());
			var maxScaleDomain = d3.max(this.scale.domain());
			var zoomedDist = 120;

			var topLeft = {x: 0, y: (minScaleDomain)*this.constant};
			var	topRight = {x: zoomedDist, y: this.chrHeight}; 
			var bottomLeft = {x: 0, y: (maxScaleDomain)*this.constant}; 
			var bottomRight = {x: zoomedDist, y:0};

			if (this.orientation == mapViewer.OrientationEnum.RIGHT) {
				topRight.y = (minScaleDomain)*this.constant;
				topLeft.y = this.chrHeight;
				bottomRight.y = (maxScaleDomain)*this.constant;
				bottomLeft.y = 0;
			}
	    	// draw the zoom polygon
	    	var polygon = svgParent.append("polygon")
	    		.attr("id", "polygon").attr("class", "polygon")
	    		.attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
	    		.style("fill","transparent").style("stroke-width", .5)
	    		.style("stroke", "#696969")
    			.attr("points", topLeft.x + "," + topLeft.y + " " + bottomLeft.x + "," + bottomLeft.y + " " + 
    							topRight.x + "," + topRight.y + " " + bottomRight.x + "," + bottomRight.y );
			this.svg = polygon;

			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}
		
		update(svgParent) {
			svgParent.selectAll("#polygon").remove(); 
			this.draw(svgParent);
			return svgParent;
		}

		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},

	Chr: class {
		constructor(orientation, linkageGroup, chrRect, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.linkageGroup = linkageGroup;
			this.chrRect = chrRect;
			this.markerEvents = markerEvents;
			this.rect = new mapViewer.Rectangle(orientation, mapViewer.RectangleEnum.CHR, chrRect);
			this.markersOnRect = new mapViewer.MarkersOnRect(orientation, 0, mapViewer.RectangleEnum.CHR, linkageGroup, chrRect, this.markerEvents);
			this.brush = new mapViewer.Brush(orientation, linkageGroup, chrRect, this.markerEvents);
			this.markersOnRectTranslateX = 0;
			this.markersOnRectTranslateY = 0;

		}

		calculateWidth(svgParent) {
			svgParent.selectAll("#Chr").remove(); 
			this.draw(svgParent);
    		var width = svgParent.selectAll("#Chr").node().getBBox().width;
			svgParent.selectAll("#Chr").remove(); 
			return width;
		}

		draw(svgParent) {
			
			this.drawChr(svgParent);

			var tmpTranslateX = 0;
			var tmpTranslateY = 0;
	
			this.rect.setTranslate(tmpTranslateX, tmpTranslateY);

			this.rect.draw(this.svg);
			this.markersOnRect.setTranslate(tmpTranslateX, tmpTranslateY);
			
			this.markersOnRect.draw(this.svg);
			this.brush.setTranslate(0,0);

			this.brush.draw(this.svg);

			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}
		
		update(svgParent) {
			this.brush.update(svgParent);
			return svgParent;
		}
		
		onForceTick(svgParent){
			this.markersOnRect.onForceTick(svgParent);
		}

		drawChr(svgParent) {

	        var Chr = svgParent.append("g")
    		.attr("id", "Chr").attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
    		.attr("visibility", "unhidden");

	        this.svg = Chr;
			
			return this.svg;
		}

		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},

	Zoom: class {
		constructor(orientation, showMarkerPos, linkageGroup, chrRect, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.chrRect = chrRect;
			this.markerEvents = markerEvents;
			this.showMarkerPos = showMarkerPos;
			this.rect = new mapViewer.Rectangle(orientation, mapViewer.RectangleEnum.ZOOM, chrRect);
			
			var markerOnRectOrientation = mapViewer.OrientationEnum.LEFT;
			if (this.orientation == mapViewer.OrientationEnum.LEFT) {
				// for the zooomed view, the marker labels face the opposite direction to the chr labels
				markerOnRectOrientation = mapViewer.OrientationEnum.RIGHT;
			}				
			this.markersOnRect = new mapViewer.MarkersOnRect(markerOnRectOrientation, showMarkerPos, mapViewer.RectangleEnum.ZOOM, linkageGroup, chrRect, this.markerEvents);

		}
		
		calculateWidth(svgParent) {
			svgParent.selectAll("#Zoom").remove(); 
			this.draw(svgParent);
    		var width = this.svg.node().getBBox().width;
			svgParent.selectAll("#Zoom").remove(); 
			return width;
		}

		draw(svgParent) {

			this.drawZoom(svgParent);
	
			var tmpTranslateX = 0;
			var tmpTranslateY = 0;

			if (this.orientation == mapViewer.OrientationEnum.RIGHT) { 
				tmpTranslateX = this.markersOnRect.calculateWidth(this.svg) - 20;
				if (this.showMarkerPos) {
					tmpTranslateX -= 30;
				}
			}
			this.rect.setTranslate(tmpTranslateX, tmpTranslateY);
			
			this.rect.draw(this.svg);
			this.markersOnRect.setTranslate(tmpTranslateX, tmpTranslateY);
			
			this.markersOnRect.draw(this.svg);
			
			// set the zoom dimensions before assigning the parent (which contains containers drawn preceding it)
			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;
		
			return svgParent;
		}

		update(svgParent) {
			svgParent.selectAll("#Zoom").remove(); 
			this.draw(svgParent);
			return svgParent;
		}

		onForceTick(svgParent){
			this.markersOnRect.onForceTick(svgParent);
		}
	
		drawZoom(svgParent) {
	        var Zoom = svgParent.append("g")
			.attr("id", "Zoom").attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
			.attr("visibility", "unhidden");
	
	        this.svg = Zoom;
			
			return this.svg;
		}
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},

	Rectangle: class {
		constructor(orientation, type, chrRect) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.type = type;
			this.chrWidth = chrRect.width;
			this.chrHeight = chrRect.height;
		}

		draw(svgParent) {

			var Rect = svgParent.append("rect");
	        Rect.attr("width", this.chrWidth)
	        	.attr("id", "Rect").attr("x", this.translateX)
	        	.style("stroke-width", .5);

			if (this.type == mapViewer.RectangleEnum.CHR) {
				// Chromosome with rounded telomere ends
				this.translateY -= 18;
		        Rect.attr("y", this.translateY)
		        	.attr("height", this.chrHeight + 34)
					.attr("rx", 13).attr("ry", 18) 
					.style("fill", "#C89696").attr("stroke", "black");
			}
			else {
				// Zoomed view with flat end
		        Rect.attr("y", this.translateY)
					.attr("height", this.chrHeight)
		        	.attr("rx", 0).attr("ry", 1) 
	        		.style("fill", "#FFDCDC").attr("stroke", "black");
			}
			this.svg = Rect;
		
			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}
		
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}
		
	},

	MarkersOnRect: class {
		constructor(orientation, showMarkerPos, type, linkageGroup, chrRect, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.linkageGroup = linkageGroup;
			this.chrRect = chrRect;
			this.markerEvents = markerEvents;
			this.type = type;
			this.showMarkerPos = showMarkerPos;
			var map_lg_name = this.linkageGroup.mapId+"_"+this.linkageGroup.id+"_"+this.type;
			this.marker_lines_className = "marker_lines_"+map_lg_name;
    		this.lines_to_marker_className = "lines_to_marker_"+map_lg_name;
    		this.marker_names_className = "marker_names_"+map_lg_name;
    		this.marker_pos_className = "marker_pos_"+map_lg_name;
    		this.marker_pos_lines_className = "marker_pos_lines_"+map_lg_name;
    		
    		this.scale = "";
			if (type == mapViewer.RectangleEnum.CHR) {
				this.scale = this.markerEvents.chrScaleRange;
			}
			else { 
				// these markers are on the zoomed rect 
				this.scale = this.markerEvents.chrScaleRange;
			}
		}
		
		calculateWidth(svgParent) {
			svgParent.selectAll("#MarkersOnRect").remove(); 
			var all = true;
			var _allNodes = this.markerEvents.getFilteredNodes("displayOnRect", this.type, all); 
			this.drawHelper(svgParent, _allNodes);
    		var width = this.svg.node().getBBox().width;
			svgParent.selectAll("#MarkersOnRect").remove(); 
			return width;
		}
		
		draw(svgParent) {
			var _nodes = this.markerEvents.getFilteredNodes("displayOnRect", this.type); 
			this.drawHelper(svgParent, _nodes);
		}
		
		drawHelper(svgParent, _nodes) {

			this.drawMarkersOnRect(svgParent);
			
			var labelStartPosX = this.chrRect.width;  // default: labels are to the right of the rectangle
			var labelEndPosX = this.chrRect.width*2;
			var textAnchor = "start"; 
    		var markerPosLinesStartX = 0;
    		var markerPosLinesEndX = -5;
    		var markerPosTextAnchor = "end";
	    	
			if (this.orientation == mapViewer.OrientationEnum.LEFT) {
				// the labels are to the left of the rectangle
				labelStartPosX = 0;
				labelEndPosX = -this.chrRect.width;
				textAnchor = "end";
	    		markerPosLinesStartX = this.chrRect.width;
	    		markerPosLinesEndX = this.chrRect.width+5;
	    		markerPosTextAnchor = "start";

			}

	    	var labelOffsetEndPosY = 0;
	    	if (this.type == mapViewer.RectangleEnum.CHR) {
	    		labelOffsetEndPosY = -8;
	    	}

	    	// draw all markers lines on rect. 
	    	var _markerLineAllNodes = this.markerEvents.getFilteredNodes("displayOnRect", mapViewer.RectangleEnum.ZOOM); 

			// Draw marker lines 
			var marker_lines = this.svg.append("g");
    		marker_lines.attr("class", this.marker_lines_className)
	    		.attr("transform", "translate(0,0)")
	    		.selectAll("line") 
	    		.data(_markerLineAllNodes)
	    		.enter().append("line")
	    		.attr("class", this.marker_lines_className )
	    		.attr("x1", 0)
	    		.attr("y1", function(d) { return d.y_init; })
	    		.attr("x2", this.chrRect.width)
	    		.attr("y2", function(d) { return d.y_init; }) 
	    		.style("stroke", function(d) { return d.marker.color; })
	    		.style("stroke-width", 4);
    		

	    	// draw lines to marker names beside view. use restricted set if type is CHR
    		var lines_to_marker = this.svg.append("g");
		    lines_to_marker.attr("class", this.lines_to_marker_className)
	    		.attr("transform", "translate(0,0)")
	    		.selectAll("line")
	    		.data(_nodes) // if chr, then restrict density of marker labels
	    		.enter().append("line")
	    		.attr("class", this.lines_to_marker_className)
	    		.attr("x1", labelStartPosX ).attr("y1", function(d) { return d.y_init; })
	    		.attr("x2", labelEndPosX ).attr("y2", function(d) { return d.y + labelOffsetEndPosY; })
	    		.style("stroke", function(d) {return tripalMap.convertTextColor(d.marker.color);})
	        	.style("stroke-width", 1);
    		
	    	// position the marker names beside view
		    var _this = this;
			var marker_names = this.svg.append("g");
	    	marker_names.attr("class", this.marker_names_className)
	    		.attr("transform", "translate(0,0)")
				.selectAll("text")
	    		.data(_nodes)
				.enter().append("text")
	    		.attr("class", this.marker_names_className)
	    		.attr("id", function(d) { return _this.marker_names_className+"_"+ encodeURIComponent(d.name); }) 
	    		.attr("x", labelEndPosX )
	    		.attr("y", function(d) { return d.y + labelOffsetEndPosY; })
	    		.text(function(d) { return d.name; })
	    		.style("fill", function(d) { return tripalMap.convertTextColor(d.marker.color); })
	    		.style("text-anchor", textAnchor); 
	    	
	    	if (this.type == mapViewer.RectangleEnum.ZOOM) {
    		
	    		// only display tooltips on zoomed rect
    			d3.selectAll("#tooltip_"+_this.marker_lines_className).remove(); 
    			var tip = d3.select("body").append("div")   
            		.attr("id", "tooltip_"+_this.marker_lines_className).attr("class", "TripalMap_tooltip")               
            		.style("opacity", 0);

    			var _this = this;
    			marker_lines.selectAll("#MarkersOnRect line."+_this.marker_lines_className)
    				.on("mouseover", function(d) { 
    					_this.markerEvents.mouseover(tip, d, _this.linkageGroup.name, _this.linkageGroup.mapName, _this.svg);
    				})
    				.on("mouseout", function(d) { _this.markerEvents.mouseout(tip, _this.svg) })
    				.on("click", function(d) { _this.markerEvents.onclick(d) });
    			
    			marker_names.selectAll("#MarkersOnRect text."+_this.marker_names_className)
					.on("mouseover", function(d) { 
						_this.markerEvents.mouseover(tip, d, _this.linkageGroup.name, _this.linkageGroup.mapName, _this.svg);
					})
					.on("mouseout", function(d) {_this.markerEvents.mouseout(tip, _this.svg)})
					.on("click", function(d) { _this.markerEvents.onclick(d) });
			
    			// only show marker position on zoom
	    	    if (this.showMarkerPos) {

	    	    	var marker_pos = this.svg.append("g");
	    	    	marker_pos.attr("class", this.marker_pos_className)
	        			.attr("transform", "translate(0,0)")
	        			.selectAll("text."+this.marker_pos_className)
	        			.data(_markerLineAllNodes)
	        			.enter().append("text")
	        			.attr("class", this.marker_pos_className)
	        			.attr("x", function(d) { return markerPosLinesEndX; }).attr("y", function(d) { return d.y; })
	        			.text(function(d) { return d.marker.pos; })
	        			.style("fill", '#333') 
	        			.style("text-anchor", markerPosTextAnchor); 

	        		var marker_pos_lines = this.svg.append("g");
	    	    	marker_pos_lines.attr("class", this.marker_pos_lines_className)
	        			.attr("transform", "translate(0,0)")
	        			.selectAll("line")
	        			.data(_markerLineAllNodes)
	        			.enter().append("line")
	        			.attr("class", this.marker_pos_lines_className)
	        			.attr("x1", function(d) { return markerPosLinesStartX; }).attr("y1", function(d) { return d.y_init; })
	        			.attr("x2", function(d) { return markerPosLinesEndX; }).attr("y2", function(d) { return d.y; })
    	    			.style("stroke", function(d) {return '#333';})
    	    			.style("stroke-width", 1);
	        	}
        	
    		}

	    	this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}
		
		onForceTick(svgParent) {
	    	var tmpLabelBoundTop = -20;
	    	var tmpLabelBoundBottom = this.chrRect;

	    	var _nodes = this.markerEvents.getFilteredNodes("displayOnRect", this.type); 
	    	var _this = this;

	    	// update lines to marker names position beside view
			svgParent.selectAll("#MarkersOnRect line."+this.lines_to_marker_className)
	    	.attr("y2", function(d, i) { return _nodes[i].y; });  
	
			// update marker name position beside view
			svgParent.selectAll("#MarkersOnRect text."+this.marker_names_className) 
				.attr("y", function(d, i) { 
					_nodes[i].y = (_nodes[i].y < tmpLabelBoundTop) ? tmpLabelBoundTop : 
					(_nodes[i].y > tmpLabelBoundBottom) ? tmpLabelBoundBottom : _nodes[i].y; 
			    d.y = _nodes[i].y;
			    return d.y; 
			});

			// update lines to marker position values
			svgParent.selectAll("#MarkersOnRect line."+this.marker_pos_lines_className)
	    		.attr("y2", function(d, i) { return _nodes[i].y; });  

			// update marker position value positions
			svgParent.selectAll("#MarkersOnRect text."+this.marker_pos_className)
				.attr("y", function(d, i) { 
					_nodes[i].y = (_nodes[i].y < tmpLabelBoundTop) ? tmpLabelBoundTop : 
					(_nodes[i].y > tmpLabelBoundBottom) ? tmpLabelBoundBottom : _nodes[i].y; 
					d.y = _nodes[i].y;
					return d.y; 
				});

		}
		
		drawMarkersOnRect(svgParent) {
			
	        var MarkersOnRect = svgParent.append("g")
			.attr("id", "MarkersOnRect").attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
			.attr("visibility", "unhidden");
	
	        this.svg = MarkersOnRect;
			
			return this.svg;
		}

		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},

	QTL: class {
		constructor(orientation, type, linkageGroup, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.linkageGroup = linkageGroup;
			this.markerEvents = markerEvents;
			this.type = type; // Chr or Zoom
			var map_lg_name = this.linkageGroup.mapId+"_"+this.linkageGroup.id+"_"+this.type;
    		this.marker_rect_className = "QTL_marker_rect_"+map_lg_name;
			this.marker_lines_className = "QTL_marker_lines_"+map_lg_name;
    		this.lines_to_marker_className = "QTL_lines_to_marker_"+map_lg_name;
    		this.marker_names_className = "QTL_marker_names_"+map_lg_name;
    		this.scale = "";
			if (type == mapViewer.RectangleEnum.CHR) {
				this.scale = this.markerEvents.chrScaleRange;
			}
			else { 
				// these qtls are on the zoom
				this.scale = this.markerEvents.chrScaleRange;
			}

		}

		calculateWidth(svgParent) {
			svgParent.selectAll("#QTLs").remove(); 
			var all = true;
			this.drawHelper(svgParent, all); 
    		var width = svgParent.selectAll("#QTLs").node().getBBox().width; 
			svgParent.selectAll("#QTLs").remove();
			return width;
		}
		
		draw(svgParent) {
			var all = false;
	    	this.drawHelper(svgParent, all); 
		}
		drawHelper(svgParent, all) { 
			this.drawQTLs(svgParent);
			
			// Draw QTL rectangles in lane(s)
			// for each lane:
			var strokeWidth = 6;
			var labelLineLen = 20;
			var labelStartPosX = strokeWidth;  // default: labels are to the right of the rectangle
			var labelEndPosX = strokeWidth+labelLineLen;
			var textAnchor = "start"; 
			var mirrorPosX = 1;
			
			//var orientLeft = false;
			if (this.orientation == mapViewer.OrientationEnum.LEFT) {
				// the labels are to the left of the rectangle
				mirrorPosX = -1;
				textAnchor = "end";
				labelStartPosX = 0;
			}
			
			// Calculate max possible character width
			var laneOffset = labelLineLen;// + strokeWidth;
			var textLenTest = this.svg.selectAll("text").data(["0"]).enter().append("text").text("W"); // Use the widest char 
			var chrLen = textLenTest.node().getBBox().width;
			textLenTest.remove();
			
	    	var labelOffsetEndPosY = 0;
	    	var orientChr = false;
	    	if (this.type == mapViewer.RectangleEnum.CHR) {
	    		orientChr = true;
	    		laneOffset = 10;
	    		labelOffsetEndPosY = -8;
	    	}

	    	// draw all QTL rects. 
	    	var _markerLineAllNodes = this.markerEvents.getFilteredNodes("displayOnQTL", mapViewer.RectangleEnum.ZOOM, all); 

			// Draw rects 
			var marker_lines = this.svg.append("g");
    		marker_lines.attr("class", this.marker_rect_className)
	    		.attr("transform", "translate(0,0)")
	    		.selectAll("rect")
	    		.data(_markerLineAllNodes)
	    		.enter().append("rect")
	    		.attr("class", this.marker_rect_className )
	    		.attr("width", strokeWidth)
	        	.attr("x", function(d) { return ((d.x - 1) * laneOffset + (orientChr ? 0 : d.qtlMaxChrs*chrLen)) * mirrorPosX;})
		        .attr("y", function(d) { return d.y_init;})
		        .attr("height", function(d) { return d.height; })
				.attr("rx", 0).attr("ry", 1) 
				.style("fill",  function(d) {return tripalMap.convertTextColor(d.marker.color);})
				.style("stroke-width", .5)
				.style("stroke", "black");

	    	// draw lines to marker names beside view
	    	var _nodes = this.markerEvents.getFilteredNodes("displayOnQTL", this.type, all); 

    		var lines_to_marker = this.svg.append("g");
		    lines_to_marker.attr("class", this.lines_to_marker_className)
	    		.attr("transform", "translate(0,0)")
	    		.selectAll("line")
	    		.data(_nodes)
	    		.enter().append("line")
	    		.attr("class", this.lines_to_marker_className)
	    		.attr("x1", function(d) { return (((d.x - 1) * laneOffset) + labelStartPosX + (orientChr ? 0 : d.qtlMaxChrs*chrLen)) * mirrorPosX;} )
	    		.attr("y1", function(d) { return d.y_init; })
	    		.attr("x2", function(d) { return (((d.x - 1) * laneOffset) + labelEndPosX + (orientChr ? 0 : d.qtlMaxChrs*chrLen)) * mirrorPosX; } )
	    		.attr("y2", function(d) { return d.y + labelOffsetEndPosY; })
	    		.style("stroke", function(d) {return tripalMap.convertTextColor(d.marker.color);})
	        	.style("stroke-width", 1);

	    	// position the marker names beside view
		    var _this = this;
			var marker_names = this.svg.append("g");
	    	marker_names.attr("class", this.marker_names_className)
	    		.attr("transform", "translate(0,0)")
				.selectAll("text")
	    		.data(_nodes)
	    		.enter().append("text")
	    		.attr("class", this.marker_names_className)
	    		.attr("id", function(d) { return _this.marker_names_className+"_"+ encodeURIComponent(d.name); }) 
	    		.attr("x", function(d) { return (((d.x -1) * laneOffset) + labelEndPosX + (orientChr ? 0 : d.qtlMaxChrs*chrLen)) * mirrorPosX; })
	    		.attr("y", function(d) { return d.y + labelOffsetEndPosY; })
	    		.text(function(d) { return d.name; })
	    		.style("fill", function(d) { return tripalMap.convertTextColor(d.marker.color); })
	    		.style("text-anchor", textAnchor); 

	    	// Tooltips: hoverover and on marker click
    		if (this.type == mapViewer.RectangleEnum.ZOOM) {
    			// only display tooltips on zoom
    			d3.selectAll("#tooltip_"+_this.marker_lines_className).remove(); 
    			var tip = d3.select("body").append("div")   
            	.attr("id", "tooltip_"+_this.marker_lines_className).attr("class", "TripalMap_tooltip")               
            	.style("opacity", 0);

    			var _this = this;
    			marker_lines.selectAll("#QTLs line."+_this.marker_lines_className)
    				.on("mouseover", function(d) { 
    					_this.markerEvents.mouseover(tip, d, _this.linkageGroup.name, _this.linkageGroup.mapName, _this.svg);
    				})
    				.on("mouseout", function(d) {_this.markerEvents.mouseout(tip, _this.svg)})
    				.on("click", function(d) { _this.markerEvents.onclick(d) });
    			
    			marker_names.selectAll("#QTLs text."+_this.marker_names_className)
					.on("mouseover", function(d) { 
						_this.markerEvents.mouseover(tip, d, _this.linkageGroup.name, _this.linkageGroup.mapName, _this.svg);
					})
					.on("mouseout", function(d) {_this.markerEvents.mouseout(tip, _this.svg)})
					.on("click", function(d) { _this.markerEvents.onclick(d) });
			
    		}

			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}

		update(svgParent) {
			svgParent.selectAll("#QTLs").remove(); 
			this.draw(svgParent);
			return svgParent;
		}

		onForceTick(svgParent) {
			var tmpLabelBoundTop = -20;
	    	var tmpLabelBoundBottom = this.chrRect;

	    	var _nodes = this.markerEvents.getFilteredNodes("displayOnQTL", this.type); 
			var _this = this;

			// update lines to marker names position beside view
			svgParent.selectAll("#QTLs line."+this.lines_to_marker_className)
				.attr("y2", function(d, i) { return _nodes[i].y; }); 

			// update marker name position beside view
			svgParent.selectAll("#QTLs text."+this.marker_names_className)
				.attr("y", function(d, i) { 
					_nodes[i].y = (_nodes[i].y < tmpLabelBoundTop) ? tmpLabelBoundTop : 
					(_nodes[i].y > tmpLabelBoundBottom) ? tmpLabelBoundBottom : _nodes[i].y;
					d.y = _nodes[i].y;
					return d.y; 
			});
			
		}
		
		drawQTLs(svgParent) {
			
	        var QTLs = svgParent.append("g")
			.attr("id", "QTLs").attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
			.attr("visibility", "unhidden");
	        this.svg = QTLs;
			
			return this.svg;
		}

		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;

		}
	},

	Brush: class {		
		constructor(orientation, linkageGroup, chrRect, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.linkageGroup = linkageGroup;
			this.chrRect = chrRect;
			this.markerEvents = markerEvents;
			this.scale = this.markerEvents.zoomScaleRange;
			this.brushRect = new mapViewer.BrushRect(this.markerEvents);
			this.cbrushObj = 0;
		}
		draw(svgParent) {
		
	    	// chromosome brush overlay (selectable zoom area on top of chromosome)
	    	if (tripalMap.d3VersionFour()) { 
	    		this.cbrushObj = d3.brushY()
	    		.extent([[0, 0], [this.chrRect.width, this.scale[1] ]])
	    		.on("brush", this.onBrushScroll());
	    	}
	    	else {
	    		var _this = this;
	    		this.cbrushObj = d3.svg.brush()
	       			.y(_this.scale) 
	       			.on("brush", function() { _this.onBrushScroll()});
		    	this.brushRect.cBrushObj = this.cbrushObj;
	    	}
	    	    
	    	
	    	svgParent.append("g")
	    		.attr("id", "brush_id")
	    		.attr("class", "brush_class")
	    		.attr("transform", "translate("+this.translateX+","+this.translateY+")")    
	        	.call(this.cbrushObj)
	    		.selectAll("rect")
	        	// brush is wider than the chromosome width to expose the stroke border.
	    		.attr("fill", "#FFDCDC").attr("stroke-width", "1")
	    		.attr("width", this.chrRect.width + 4)
	    		.attr("x", -2); // brush offset

	    	var chrBrushSvg = svgParent.select("#brush_id");
	    	this.brushRect.draw(chrBrushSvg);
	    	
	    	this.svg = chrBrushSvg;
	    	this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}
		onBrushScroll() {
			this.brushRect.updateBrushDomain();
			this.markerEvents.onBrushScroll(this.brushRect.brushScale);
		}
		getChrBrush() {
			return this.chrBrush;
		}
		update(svgParent) {
			this.brushRect.update(svgParent);
			return svgParent;
		}
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},

	BrushRect: class {
		constructor(markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.chrRect = markerEvents.chrRect;
	    	this.markerEvents = markerEvents;
	    	this.markersMaxPos = this.markerEvents.markersMaxPos;
	    	this.constant = this.chrRect.height/this.markersMaxPos; 
	    	this.brushScale = this.markerEvents.chrScaleRange;
	    	this.initScaleRange = this.markerEvents.initScaleRange;
	    	this.zoomScaleRange = this.markerEvents.zoomScaleRange;
	    	this.cBrushObj = '';
	    	
		}
		
    	draw(svgParent) {
			if (!(this.cBrushObj)){
				return svgParent;
			}
    		
			svgParent.append("rect")
				.attr("class", "brush_rect")
				.attr("x", -2).attr("y", 0).attr("width", this.chrRect.width + 4) 
				.attr("height", d3.max(this.brushScale.domain())*this.constant)
				.attr("cursor", "crosshair") 
				.style("fill", "#FFDCDC").style("fill-opacity", .25).style("stroke-width", 1);
		
    		var brush_rect = svgParent.selectAll(".brush_rect");

    		this.svg = brush_rect;
			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
    	}
		update(svgParent) {
			svgParent.selectAll(".brush_rect").remove();
			return svgParent;
		}
    	
    	updateBrushDomain() {
			if (!(this.cBrushObj)){
				return;
			}
			if (this.cBrushObj.empty()) {
			   this.brushScale.domain(this.markerEvents.initScaleRange.domain());
			}
			else {
				this.brushScale.domain(this.cBrushObj.extent());
    		}
			return this.brushScale;
    	}
    	
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},
	
	Correspondences: class {
		constructor(orientation, linkageGroup, markerData, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.linkageGroup = linkageGroup;
			this.markerData = markerData;
			this.markerEvents = markerEvents;
			var map_lg_name = this.linkageGroup.mapId+"_"+this.linkageGroup.id+"_"+this.orientation;
			this.correspondence_lines_className = "correspondence_lines_"+map_lg_name;
		}
		
		calculateWidth(svgParent) {
			svgParent.selectAll("#Correspondences").remove(); 
			var tmpsvg = this.draw(svgParent);
    		this.width = tmpsvg.node().getBBox().width;
			svgParent.selectAll("#Correspondences").remove(); 
			return this.width;
		}

		draw(svgParent) {

		    var svg = d3.select("#select_fieldset_mapViewer_svg").selectAll("svg");
		    svgParent = svg;
			this.drawCorrespondences(svgParent);

	    	var _nodes = this.markerEvents.getFilteredNodes("correspondences", this.type); 
	    	
	    	
			// Draw correspondence lines
	    	var _this = this;
    		var correspondence_lines = this.svg.append("g");
    		correspondence_lines.attr("class", this.correspondence_lines_className)
	    		.attr("transform", "translate(0,0)")
	    		.selectAll("line")
	    		.data(_nodes)
	    		.enter().append("line")
	    		.attr("class", this.correspondence_lines_className)
	    		.attr("id", function(d) { return _this.correspondence_lines_className+"_"+ encodeURIComponent(d.name); })
	    		.attr("x1", function(d) {  
	    			return (_this.orientation == mapViewer.OrientationEnum.LEFT) ? (d.correspondences.x1 + d.correspondences.x1_width + 1) : (d.correspondences.x1 - 1); }) 
	    		.attr("y1", function(d) { return d.correspondences.y1; })
	    		.attr("x2", function(d) { 
	    			return (_this.orientation == mapViewer.OrientationEnum.LEFT) ? (d.correspondences.x2 - 1) : (d.correspondences.x2 + d.correspondences.x2_width + 1); } )
	    		.attr("y2", function(d) { return d.correspondences.y2; })
	    		.style("stroke", function(d) {return "#114ED8";})
	        	.style("stroke-width", 1);
	    	 
			this.height = this.svg.node().getBBox().height;
			this.width = this.svg.node().getBBox().width;

			return svgParent;
		}
		
		drawCorrespondences(svgParent) {
		
			var correspondences = svgParent.append("g")
				.attr("id", "Correspondences").attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
				.attr("visibility", "unhidden");
	        this.svg = correspondences;
			
			return this.svg;
		}
		
		update(svgParent) {
		    d3.select("#select_fieldset_mapViewer_svg").selectAll("#Correspondences").remove(); 
			this.draw(svgParent);
			return svgParent;
		}

		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}
	
	},

	MapLabel: class {
		constructor(orientation, mapLabel, mapId, linkageGroupName) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.mapLabel = mapLabel;
			this.mapId = mapId;
			this.linkageGroupName = linkageGroupName;
		}
		
		draw(svgParent) {
			
	        var map_id = 100; 
			var mapUrl = Drupal.settings.baseUrl+"/tripalmap_featuremap/" + this.mapId;

		    var chrMapLabelg = svgParent.append("g");
		    chrMapLabelg.attr("id", "chrMapLabel")
	        	.attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
	        	.attr("visibility", "unhidden");

		    var titleMaxWidth = 450;

		    var chrViewXlinkText = chrMapLabelg.append("text");
		    chrViewXlinkText.attr("id", "text-select").attr("dx", this.translateX)
		    	.attr("class", "mvtitle_text").attr("y", this.translateY)
				.style("font-size", "1.3em").style("line-height", "1.2em")
				.text(this.linkageGroupName + ' of map '+ this.mapLabel)
				.call(tripalMap.wrap, titleMaxWidth);

		    // obtain height of wrapped map and linkage group title text
	    	var mvTitleText = chrViewXlinkText;
	    	var padding = 8;
	    	var mvTitleTextHeight = mvTitleText.node().getBBox().height + padding;
			
		    chrMapLabelg.append("text")
				.attr("id", "text-select2").attr("dx", this.translateX).attr("dy", this.translateY + mvTitleTextHeight)
				.style("font-size", "1.2em").style("font-style", "italic")
				.text(Drupal.t('To zoom in, drag the mouse across the linkage group'));	  

		    var svgrectRight = chrMapLabelg.selectAll(".mvtitle_text").node().getBoundingClientRect().right - svgParent.node().getBoundingClientRect().x + 22;
		    var button = chrMapLabelg.append("g");
			button.attr("id", "buttonMapLabel")
	    		.attr("transform", "translate(" + svgrectRight + "," + (this.translateY ) + ")");

	    	var bbox = button.node().getBBox();
	    	var rect = button.append("rect");
	    	rect.attr("x", bbox.x - 12)
	        .attr("y", bbox.y - 20)
	        .attr("width", 100 )
	        .attr("height", 30 )
	        .style("fill", "#f5f5f5").style("stroke-width", "1px").style("stroke", "#e3e3e3");

	    	var buttonurl = button.append("a");
	        buttonurl.attr("xlink:href", mapUrl)
	    	var buttonurltext = buttonurl.append("text");
	    	buttonurltext.style("font-size", "1.3em").style("line-height", "1.2em").text("Map Detail");
 
	    	this.svg = chrMapLabelg;

	    	this.height = this.svg.node().getBBox().height;
	    	this.width = this.svg.node().getBBox().width;
			return svgParent;
		}
		
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}
	
	},

	Ruler: class {
		constructor(orientation, type, markerEvents) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.orientation = orientation;
			this.type = type;
			this.markerEvents = markerEvents;
			//this.scale = "";
			this.scale = this.markerEvents.chrScaleRange; 

			if (this.type != mapViewer.RectangleEnum.CHR) {
				this.scale = this.markerEvents.chrScaleRange; 
			}
			else {
				this.scale = this.markerEvents.zoomScaleRange; 
			}
			this.rulerY = "";

			// set ruler orientation and axis
			if (this.orientation == mapViewer.OrientationEnum.LEFT) { 
				if (tripalMap.d3VersionFour()) { 
					this.rulerY = d3.axisLeft(this.scale).tickSize(1);
		 	    }
		 	    else {
		 	    	this.rulerY = d3.svg.axis().scale(this.scale).orient("left").tickSize(1);
		 	  	}
			}
			else {
				if (tripalMap.d3VersionFour()) { 
					this.rulerY = d3.axisRight(this.scale).tickSize(1); 
		 	    }
		 	    else {
		 	    	this.rulerY = d3.svg.axis().scale(this.scale).orient("right").tickSize(1);
		 	  	}
			}
		}
		
		calculateWidth(svgParent) {
			svgParent.selectAll("#chr_y_ruler").remove(); 
			this.draw(svgParent);
    		this.width = this.svg.node().getBBox().width;
			svgParent.selectAll("#chr_y_ruler").remove(); 
			return this.width;
		}

		draw(svgParent) {
	        // now that we have the data, create ruler for chromosome, but not if this is the reference view in the Main view
	        var chrOrZoomedView = svgParent; 
	    	chrOrZoomedView.append("g") 
				.attr("id","chr_y_ruler").attr("class", "y axis").attr("width", 1) 
				.attr("height", 1).attr("transform", "translate("+this.translateX+","+this.translateY+")")
				.call(this.rulerY);
	    	
	    	this.svg = chrOrZoomedView.select("#chr_y_ruler");
	    		
			this.height = this.svg.node().getBBox().height;
				// only the width of the chr ruler is set, the zoom ruler overlaps with the polygon 
				this.width = this.svg.node().getBBox().width;
				
			return svgParent;
		}
		update(svgParent) {
			svgParent.selectAll("#chr_y_ruler").remove();
			this.draw(svgParent);
			return svgParent;
		}

		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}
	},

	Legend: class {
		constructor(markerData) {
			this.translateX = 0;
			this.translateY = 0;
			this.height = 0;
			this.width = 0;
			this.svg = 0;
			this.markerTypeColorMap = markerData.markerTypeColorMap;
			this.markerTypesWithColors = {}; // associative list of marker types and corresponding colors
			this.markerData = markerData;
		}
		
		draw(svgParent) {
			
			var markerTypesWithColors = this.setMarkerTypesWithColors();
			var colorBox = {width: 10, height: 10, outlineColor: "#696969"};
			var colorBoxPosX = this.translateX;
			var legendView = svgParent.append("g")
				.attr("id","legend")
				.attr("transform", "translate(" + this.translateX + "," + this.translateY + ")")
				.attr("visibility", "unhidden");

			for (var markerType in this.markerTypesWithColors) {
				if (markerTypesWithColors.hasOwnProperty(markerType)) {

					// marker color box
					var markerColor = markerTypesWithColors[markerType];
					legendView.append("rect")
						.attr("id","legendViewColorBox")
						.attr("x", colorBoxPosX).attr("y", -colorBox.height)
						.attr("width", colorBox.width)
						.attr("height", colorBox.height)
						.style("fill", markerColor)
						.style("stroke", colorBox.outlineColor)
						.style("stroke-width", 1);
					
					// marker text label
					var markerTypeText = markerType.replace(/['"]+/g, '');
					var fontSize = 9;
					var text = legendView.append("text")
						.attr("id", "legendViewText")
						.attr("dx", colorBoxPosX + colorBox.width + 5).attr("dy", 0)
						.style("font-size", fontSize+"pt")
						.text(Drupal.t(' @markerType  ', {'@markerType': markerTypeText}));					
					
					// calculate X position for the next marker box based on the length of preceding text
					colorBoxPosX += text.node().getBBox().width + 25;
				}
			}
			return svgParent;
		}
		
		setMarkerTypesWithColors() {
			var markerTypes = this.markerData.getMarkerTypes();
			for( var markerType in this.markerTypeColorMap) {
				if (markerTypes.has(markerType)) {
					this.markerTypesWithColors[markerType] = this.markerTypeColorMap[markerType];
				}
			}
			return this.markerTypesWithColors;
		}
		
		setTranslate(x,y) {
			this.translateX = x;
			this.translateY = y;
		}

	},
	
		
};
