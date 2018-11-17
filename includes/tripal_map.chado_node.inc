<?php

/**
 * @file
 *	 Contains Drupal hooks for interacting with nodes.
 *
 * @ingroup tripal_map
 */


/**
 * Implements hook_node_view().
 * 
 * function: tripal_map_node_view
 *	 Route nodes to display corresponding templates and toc
 *
 * @param $node 
 * @param $view_mode
 * @param $langcode
 *
 *
 * @ingroup tripal_map_includes
 */
function tripal_map_node_view($node, $view_mode, $langcode) {

  switch ($node->type) {
    case 'chado_featuremap':
      // Tripal view modes are full page view and teaser.
      if ($view_mode == 'full') {
        // Generate the Table of Contents using the default Tripal node template. 
        // When the map node type is first displayed the base template is shown first
        $node->content['#tripal_generic_node_template'] = TRUE;
        //unset($build['tripal_featuremap_base']);
        $node->content['tripal_featuremap_base'] = array( // overrides the tripal_featuremap_base
          '#theme' => 'mainlab_featuremap_base',//'tripal_featuremap_base',
          '#node' => $node,
          //tripal_toc_id uniquely references the template, used for constructing URLs for the content.
          '#tripal_toc_id'    => 'base',
          // tripal_toc_title is listed in the table of contents
          '#tripal_toc_title' => 'Map Overview',
          // set '#weight' element for the item to appear at the top of the list, default sorting is alphabetical.
          '#weight' => -100,
        );
         
        $exclude_maps = explode(";", variable_get('tripal_map_map_overview_exclude_maps'));
  		$exclude_maps = array_map('trim', $exclude_maps);
  		$map_name = $node->featuremap->name;
  
  		if (!in_array($map_name, $exclude_maps)) {
          $node->content['tripal_map_mapviewer'] = array(
            '#theme' => 'tripal_map_mapviewer',
            '#node' => $node,
            '#tripal_toc_id' => 'MapViewer',
            '#tripal_toc_title' => 'View Map',
          );
  		}
      }
      // set the content for the teaser view
      if ($view_mode == 'teaser') {
        // The teaser is also a required template
        $node->content['tripal_map_teaser'] = array(
          '#theme' => 'tripal_map_teaser',
          '#node' => $node,
         );
      }
    break;
    // you can add custom content to any node type by adding content to the node
    // in the same way as above.
    case 'chado_organism':
      if ($view_mode == 'full') {
        $node->content['tripal_organism_maps'] = array(
          '#theme' => 'tripal_organism_maps',
          '#node' => $node,
          '#tripal_toc_id'    => 'maps',
          '#tripal_toc_title' => 'maps',
        );
    }
    break;
    case 'chado_feature':
 
      if ($view_mode == 'full') {
        $node->content['mainlab_feature_genetic_marker_map_positions'] = array(
          '#theme' => 'mainlab_feature_genetic_marker_map_positions',
          '#node' => $node,
          '#tripal_toc_id'    => 'genetic_marker_map_positions',
          '#tripal_toc_title' => 'Map Positions',
        );
        $node->content['mainlab_feature_QTL_map_positions'] = array(
          '#theme' => 'mainlab_feature_QTL_map_positions',
          '#node' => $node,
          '#tripal_toc_id'    => 'QTL_map_positions',
          '#tripal_toc_title' => 'Map Positions',
        );
        $node->content['mainlab_feature_heritable_phenotypic_marker_map_positions'] = array(
          '#theme' => 'mainlab_feature_heritable_phenotypic_marker_map_positions',
          '#node' => $node,
          '#tripal_toc_id'    => 'heritable_phenotypic_marker_map_positions',
          '#tripal_toc_title' => 'Map Positions',
        );
      }
    break;

  }
}

