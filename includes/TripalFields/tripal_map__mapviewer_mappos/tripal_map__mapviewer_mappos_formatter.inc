<?php

/**
 * @file
 *	 Creates field formatter class for the tripal map marker map positions field
 *
 * @ingroup tripal_map
 */


class tripal_map__mapviewer_mappos_formatter extends ChadoFieldFormatter {
        
  // The default label for this field.
  public static $default_label = 'MapViewer Marker Map Positions';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('tripal_map__mapviewer_mappos');

  // The list of default settings for this formatter.
  public static $default_settings = array(
  	'title' => 'MapViewer Marker Map Positions',	
    'empty' => 'There are no marker map positions',
  );

  
  /**
   * Implements the hook_field_formatter_settings_form()
   *  
   * function: settingsForm
   *   Provides the field's setting form appearing on the 'Manage Display' page of the content
   *   type admin page. 
   *
   * @param $view_mode
   *   The view mode being configured.
   *  
   * @param $form 
   *   The (entire) configuration form array, which will usually have no use
   *   here.  Typically for reference only.
   *
   * @param &$form_state
   *   The form state of the (entire) configuration form.
   *
   * @return $element
   *   A Drupal Form array containing the settings form for this field.
   *
   * @ingroup tripal_map_includes
   */
  public function settingsForm($view_mode, $form, &$form_state) {

    $display = $this->instance['display'][$view_mode];
    $settings = $display['settings'];
    $element = array();
    $element['title'] = array(
    		'#type' => 'textfield',
    		'#title' => 'Table Header',
    		'#default_value' => array_key_exists('title', $settings) ? $settings['title'] : 'Relationship',
    );
    $element['empty'] = array(
    		'#type' => 'textfield',
    		'#title' => 'Empty text',
    		'#default_value' => array_key_exists('empty', $settings) ? $settings['empty'] : 'There are no marker map positions',
    );
    
    return $element;
  }

  
  /**
   * Implements the hook_field_formatter_settings_summary()
   *
   * function: settingsSummary
   *   Provides a summary of the formatter settings appearing on the 'Manage Display' page of the content
   *   type admin page.
   *
   * @param $view_mode
   *   The view mode being configured.
   *
   * @return $summary
   *   A string description field settings summary
   */
  public function settingsSummary($view_mode) {
  	 
   $display = $this->instance['display'][$view_mode];
   $settings = $display['settings'];

   $summary = t('Title: @title<br>Empty: @empty',
     array(
      '@title' => $settings['title'],
      '@empty' => $settings['empty'])
     );

    return $summary;

  }

  /**
   * Implements the hook_field_formatter_view()
   *
   * function: view
   *   Provides the display for the marker map positions field when viewed on the web page. 
   *
   *  @param $element
   *  @param $entity_type
   *  @param $entity
   *  @param $langcode
   *  @param $items
   *  @param $display
   *
   *  @return 
   *   Populate A Drupal element array containing the mapviewer marker map positions table. 
   *   Restrict this to the contents of $items[$delta]['values] array.
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {

    // Get the settings
    $settings = $display['settings'];
    $field_name = $this->field['field_name'];
    $rows = array();

    $headers = array('Map Name', 'Linkage Group', 'Position', 'Locus', 'MapViewer');
    $rows = array();
    
    foreach ($items as $delta => $item) {
      $markers = $item['value'];
      if (!$markers) {
        continue;
      }
      foreach($markers as $marker) {
        // Get the field values
        $map_name = $marker['map_name'];
        $linkage_group = $marker['linkage_group'];
        $position = $marker['marker_pos'];
        $locus = $marker['feature_name'];
        $mapviewer = "<a href= '/mapviewer/".$marker['map_id']."/".$linkage_group."/".$locus."' target='_blank'>MapViewer</a>";
        $rows[] = array($map_name, $linkage_group, $position, $locus, $mapviewer);
      }
    }
    
    $table = array(
      'header' => $headers,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'tripal-map--mapviewer-marker-positions-table',
        'class' => 'tripal-data-table'
      ),
      'sticky' => FALSE,
      'caption' => "",
      'colgroups' => array(),
      'empty' => 'There are no marker positions available.',
    );
    
    $content = theme_table($table);
    if (count($items) > 0) {
    	// once table array structure is defined, call Drupal theme_table() to generate the table.
    	$element[0] = array(
    			'#type' => 'markup',
    			'#markup' => $content,
    	);
    }
    
    return;
   
  }

}
