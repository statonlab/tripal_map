
/* 
 * File: tripal_dotplot.js
 * Obtain settings from MapViewer PHP form, call Dot Plot draw to display selected chromosomes.
 */

(function($) {
  Drupal.behaviors.tripal_map_dotplot_Behavior = {
    attach: function (context, settings) {
    	
    	$('#select_fieldset_dotplot').once('select_fieldset_dotplot', function() {

    	var primMap =     Drupal.settings.mapDotPlotJS.primary_map;
    	var primLinkageGroup =   Drupal.settings.mapDotPlotJS.prim_linkage_group; 
    	var primGeneticFeatures = Drupal.settings.mapDotPlotJS.dotplot_prim_genetic_features;

    	var secMap =       Drupal.settings.mapDotPlotSecJS.secondary_map;                   	
       	var secLinkageGroup =  Drupal.settings.mapDotPlotSecJS.secondary_linkage_group;
    	var secGeneticFeatures =  Drupal.settings.mapDotPlotSecJS.dotplot_sec_genetic_features;

    	var featuresCombined = primGeneticFeatures.concat(secGeneticFeatures);
    	
    	var container =  "#select_fieldset_dotplot";
    	$(container).before('<div></div><div id ="select_fieldset_dotplot_svg" width="100%"></div>');

    	var corres = tripalMap.findCorrespondences(featuresCombined);
    	dotplot.drawDotplotPlotly(primMap, primLinkageGroup, secMap, secLinkageGroup, corres);

    });
    }
  };
})(jQuery);

