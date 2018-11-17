/*
 * Obtain settings from Drupal PHP form, call map overview draw function to draw chromosome glyphs
 */

(function($) {
  Drupal.behaviors.tripal_mapOverviewBehavior = {
    attach: function (context, settings) {
  
        $('#select_fieldset_genetic_map_overview').once('select_fieldset_genetic_map_overview', function() {

        // obtain the featuremap from the form argument 
        var geneticFeatures =         Drupal.settings.mapOverviewDisplayJS.mapOverview_genetic_features;
        var mapName =                 Drupal.settings.mapOverviewDisplayJS.reference_map_name; 
        var mapType =                 Drupal.settings.mapOverviewDisplayJS.reference_map_type; 
        var featuremapId = 			  Drupal.settings.mapOverviewDisplayJS.featuremap_id;
        var markerTypeDisplayStates = Drupal.settings.mapOverviewDisplayJS.marker_type_display_states;
        var markerTypeColorMap =      Drupal.settings.mapOverviewDisplayJS.marker_type_color_map;
 
        // using the map name and linkage group, create the panes link in the toc
        tripalMap.orderFeaturemapToc(featuremapId);     	
 
        // be very careful when creating a unique list of markers as some have the same name, but different position. 
        var features = {};
        geneticFeatures.forEach(function(feature) {
            if (!(JSON.stringify(feature) in features)) {
                features[JSON.stringify(feature)] = feature;
            }
        });

        var uniqueGeneticMarkers = [];
        for(var key in features) {
            uniqueGeneticMarkers.push(features[key]);
        } 

        var strUniqueGeneticMarkers = JSON.stringify(uniqueGeneticMarkers);
        var strMarkerTypeDisplayStates = "";
		if (Object.keys(markerTypeDisplayStates).length > 0) {
            strMarkerTypeDisplayStates = JSON.stringify(markerTypeDisplayStates);
        }
		
        var displayConfig = {
            markerTypeDisplayStates: strMarkerTypeDisplayStates, 
            mapName: mapName,
            mapType: mapType,
            featuremapId: featuremapId,
            colorMap: markerTypeColorMap};

        geneticOverviewMap.mapOverviewDraw(strUniqueGeneticMarkers, displayConfig);

    });
    }
  };
})(jQuery);
