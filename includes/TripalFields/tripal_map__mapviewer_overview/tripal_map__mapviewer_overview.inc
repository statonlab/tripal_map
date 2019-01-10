<?php
/**
 * @file
 *	 Creates field class for the tripal map mapviewer overview field
 *
 * @ingroup tripal_map
 */

class tripal_map__mapviewer_overview extends ChadoField {

  // Editable static constants - used by the static functions to provide information to Drupal about
  //  the field and it's default widget and formatter.
    
  public static $default_label = 'MapViewer Overview';
  public static $default_description = 'MapViewer Overview displays linkage groups as glyphs';
  public static $default_widget = 'tripal_map__mapviewer_overview_widget';
  public static $default_formatter = 'tripal_map__mapviewer_overview_formatter';
  public static $module = 'tripal_map'; // module that manages the field.

  // Global settings accessed within the globalSettingsForm, updated for all fields when the globalSettingsForm is submitted.
  public static $default_settings = array(
    'storage' => 'field_chado_storage',
    'searchable_keys' => array(),
  );

  // Instance specific settings accessible within the instanceSettingsForm.
  public static $default_instance_settings = array(
    'term_vocabulary' => 'tripal_map',      // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_name' => 'mapviewer_overview',    // The unique ID or accession of the term.
    'term_accession' => 'mapviewer_overview',
    'term_fixed' => FALSE,              // allow site admin to change the term type
    'auto_attach' => FALSE,             // disable for fields that load slowly. Load asynchronously, not attaching to display or web services
    'chado_table' => 'featuremap',      // Instance maps to this Chado table.
    'chado_column' => 'featuremap_id',  // Field value comes from this Chado table column
    'base_table' => 'featuremap',
  );

  public static $no_ui = FALSE; // do not allow users to create this field or instances of this field through the UI. Instead use field_create_field() or field_create_instance().
  public static $no_data = FALSE;

  /**
   * function: load
   *    Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'];
   *   where:
   *     $entity: entity object to which this field is attached.
   *     $field_name: is the name of this field
   *     'und': language code ('und' == undefined)
   *     0: cardinality incremented by 1 when more than one item is available
   *     'value': key for the value of the field
   *
   * @ingroup tripal_map_includes
   */
  public function load($entity) {
    // ChadoFields automatically load the chado column specified in the default settings above. 
    parent::load($entity);
  
    $field_name = $this->field['field_name'];
    
    if ($entity->chado_table == 'organism') {
        
        // set default
        $entity->{$field_name}['und'][0]['value'] = NULL;
        
        // are there any featuremaps for this organism?
        $query = db_select('chado.featuremap_organism', 'fo')->fields('fo', ['featuremap_id']);
        $query->join('chado.featuremap', 'f', 'f.featuremap_id = fo.featuremap_id');
        $query->fields('f', ['name']);

        // only include featuremaps in the mview
        // and only use mviews if exist
        $mview1 = NULL;        
        if (db_table_exists('chado.tripal_map_genetic_markers_mview')) {
            $mview1 = db_select('chado.tripal_map_genetic_markers_mview', 'mview1')->fields('mview1', ['map_id']);
        }
        $mview2 = NULL;
        if (db_table_exists('chado.tripal_map_qtl_and_mtl_mview')) {
            $mview2 = db_select('chado.tripal_map_qtl_and_mtl_mview', 'mview2')->fields('mview2', ['map_id']);
        }
        $mviews = NULL;
        if ($mview1) {
          $mviews = $mview1;
          if ($mview2) {
            $mviews->union($mview2);
          }
        }
        else if ($mview2) {
          $mviews = $mview2;
        }

        $query->join($mviews, 'mview', 'mview.map_id = f.featuremap_id');
        $query->condition('fo.organism_id', $entity->chado_record->organism_id);
        $featuremaps = $query->execute()->fetchAll();
        
        if (!$featuremaps) {
            unset($entity->{$field_name});
        }
        $select = [];
        foreach ($featuremaps as $featuremap) {
            $select[$featuremap->featuremap_id] = $featuremap->name;
        }
        
        $entity->{$field_name}['und'][0]['value'] = TRUE;
        $entity->{$field_name}['und'][0]['select_choices'] = $select;
    }
    
    return $entity;
    
  }
  
  
  /**
   * Theme function for the pub widget.
   *
   * @param $variables
   */
  function theme_tripal_map__mapviewer_overview_widget($variables) {
  
   	return "<p>mapviewer overview widget theme()</p>";
  }
  
  
}