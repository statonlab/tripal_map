<?php
/**
 * @file
 * Contains all field specific code outside the classes.
 */

/**
 * Implements hook_bundle_fields_info().
 *
 * Registers a field type with Drupal/Tripal. 
 * @param $entity_type -'TripalEntity' for all Tripal Content.
 * @param $bundle - Describes Tripal Entity type (Organism, Gene, etc,) hook is called for
 * 
 * @return
 *   An array of field definitions. Call tripal_refresh_bundle_fields() to trigger create 
 *   of fields for the specific bundle.
 */
function tripal_map_bundle_fields_info($entity_type, $bundle) {
  
  $fields = array();

  // Use the 'data_table' property of the bundle to determine if want the field to be associated with this bundle.
  if (isset($bundle->data_table) AND ($bundle->data_table == 'featuremap')) {

    // MapViewer Overview Field.
    //---------------------------------
    // Summarizes the mapviewer overview on the map page. 
    	
    // Then describe the field defined by that term.
    $field_name = 'tripal_map__mapviewer_overview';
    $field_type = 'tripal_map__mapviewer_overview';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );
  }

  // Marker Feature
  if (isset($bundle->data_table) AND ($bundle->data_table == 'feature')) {
    	
    // MapViewer Marker Map Positions Field.
    //---------------------------------
    // Adds the marker map position link to mapviewer on the feature/markerpage. 
    	
    // Then describe the field defined by that term.
    $field_name = 'tripal_map__mapviewer_mappos';
    $field_type = 'tripal_map__mapviewer_mappos';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
        'storage' => array(
        'type' => 'field_chado_storage',
      ),
    );
  }
  
  return $fields;
}

/**
 * Implements hook_bundle_instances_info().
 *
 * Creates a field instance of this field type on a specific Tripal Content type/bundle. 
 * @param $entity_type - 'TripalEntity' for all Tripal Content.
 * @param $bundle - type of Tripal Entity (organism, gene, etc)
 * @return
 *   An array of field instance definitions. Define the defaults for any settings you use 
 *   in your field. 
 */
function tripal_map_bundle_instances_info($entity_type, $bundle) {
  
  $instances = array();
  
  // Map.
  //===============
  if (isset($bundle->data_table) AND ($bundle->data_table == 'featuremap')) {

    // Map Overview Summary Field.
    //---------------------------------
    // Sumarizes map overview on the map page.
    $field_name = 'tripal_map__mapviewer_overview';
    $field_type = 'tripal_map__mapviewer_overview';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'MapViewer Overview',
      'description' => 'Displays MapViewer Overview linkage group glyphs',
      'required' => FALSE,
      'settings' => array(
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => 'featuremap_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'tripal_map__mapviewer_overview_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'tripal_map__mapviewer_overview_formatter',
          'settings' => array(),
        ),
      ),
    );

  }

  // Marker Feature
  if (isset($bundle->data_table) AND ($bundle->data_table == 'feature')) {

  	// MapViewer Marker Map Positions Field.
  	//---------------------------------
  	// Displays link to marker map position in the MapViewer on Features page 
  	$field_name = 'tripal_map__mapviewer_mappos';
  	$field_type = 'tripal_map__mapviewer_mappos';
  	
  	$bundle_name = $bundle->name;
  	$field_label = $field_name;
  	//tripal_ds_create_field($field_label, $field_name, $bundle_name);
  	
  	$instances[$field_name] =  array(
  	  'field_name' => $field_name,
  	  'entity_type' => $entity_type,
  	  'bundle' => $bundle->name,
  	  'label' => 'MapViewer Marker Map Positions',
  	  'description' => 'Displays link to marker map position in the MapViewer for Features 
    		(Genetic Marker, QTL, Heritable Phenotypic Marker)',
  	  'required' => FALSE,
  	  'settings' => array(
  	  'auto_attach' => FALSE,
  	  'chado_table' => $bundle->data_table,
  	  'chado_column' => 'featuremap',
  	  'base_table' => $bundle->data_table,
  	),
  	  'widget' => array(
  	  'type' => 'tripal_map__mapviewer_mappos_widget',
  	  'settings' => array(),
  	),
  	  'display' => array(
  	    'default' => array(
  	    'label' => 'hidden',
  	    'type' => 'tripal_map__mapviewer_mappos_formatter',
  	    'settings' => array(),
  	    ),
  	  ),
  	);
  }					
  					
  return $instances;
}

/**
 * Implements hook_bundle_create_user_field().
 *
 * A privileged user has the ability to add new fields to the bundle. The
 * remote__data field is allowed to be added dynamically by the user.
 * But, Drupal doesn't know how to deal with it, so this function is called
 * for any field attached to a TripalEntity bundle type. Any fields whose
 * TripalField::$module argument is set to 'tripal_ws' and that can be
 * added dynamically will result in a call to this function.
 */
function tripal_map_bundle_create_user_field($new_field, $bundle) {

  // Get the table this bundle is mapped to.
  $term = tripal_load_term_entity(array('term_id' => $bundle->term_id));
  $vocab = $term->vocab;
  $params = array(
    'vocabulary' => $vocab->vocabulary,
    'accession' => $term->accession,
  );
  
  // We allow site admins to add new chado_linker__prop fields to an entity.
  // This function will allow us to properly add them.  But at this point we
  // don't know the controlled vocabulary term.  We'll have to use the
  // defaults and let the user set it using the interface.
  
  // MapViewer Overview
  $field_type = 'tripal_map__mapviewer_overview';
  $description = 'Displays MapViewer Overview linkage group glyphs';
  tripal_map_bundle_create_user_field_helper($new_field, $bundle, $field_type, $description);
  
  // MapViewer Marker Map Positions
  $field_type = 'tripal_map__mapviewer_mappos';
  $description = 'Displays link to marker map position in the MapViewer for Features';
  tripal_map_bundle_create_user_field_helper($new_field, $bundle, $field_type, $description);
  
}


function tripal_map_bundle_create_user_field_helper($new_field, $bundle, $field_type, $description) {

  if ($new_field['type'] == $field_type) {
    $field_name = $new_field['field_name'];
    $field_type = $field_type;

    // First add the field.
    field_create_field(array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => FIELD_CARDINALITY_UNLIMITED,
      'locked' => FALSE,
      'storage' => array(
        'type' => 'field_chado_storage',
        ),
    ));
	
    // Now add the instance
    field_create_instance(array(
      'field_name' => $field_name,
      'entity_type' => 'TripalEntity',
      'bundle' => $bundle->name,
      'label' => $new_field['label'],
      'description' => $description,
      'required' => FALSE,
      'settings' => array(
        'auto_attach' => FALSE,
        'term_vocabulary' => '',
        'term_accession' => '',
        'term_name' => ''
      ),
	  'widget' => array('type' =>  $field_type . '_widget', 'settings' => array( 'display_label' => 1,),),
	  'display' => array( 'default' => array('label' => 'hidden', 'type' => $field_type . '_formatter', 'settings' => array(),),
	  ),
    ));
  }

}
