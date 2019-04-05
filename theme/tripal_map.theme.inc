<?php

/**
 * @ingroup mainlab_featuremap
 */

function tripal_map_preprocess_mainlab_featuremap_base(&$variables) {
  $featuremap = $variables['node']->featuremap;
  $num_loci = chado_query(
    "SELECT count (F.uniquename)
      FROM {featurepos} FP
      INNER JOIN {feature} F ON F.feature_id = FP.feature_id
      WHERE F.type_id = (SELECT cvterm_id
        FROM {cvterm}
        WHERE name = 'marker_locus' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN'))
        AND featuremap_id = :featuremap_id", array(':featuremap_id' => $featuremap->featuremap_id))->fetchField();
  $num_lg = chado_query(
    "SELECT count (distinct F.uniquename)
      FROM {featurepos} FP
      INNER JOIN {feature} F ON F.feature_id = FP.map_feature_id
      WHERE F.type_id = (SELECT cvterm_id
        FROM {cvterm}
        WHERE name = 'linkage_group' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'sequence'))
        AND featuremap_id = :featuremap_id", array('featuremap_id' => $featuremap->featuremap_id))->fetchField();
  $cmap_r = chado_query(
    "SELECT db.urlprefix, X.accession
      FROM {featuremap_dbxref} FD
      INNER JOIN {dbxref} X ON FD.dbxref_id = X.dbxref_id
      INNER JOIN {db} ON db.db_id = X.db_id
      WHERE featuremap_id = :featuremap_id
      ", array(':featuremap_id' => $featuremap->featuremap_id));
  $cmap = $cmap_r->fetchObject();
  $cmap_url = is_object($cmap) ? $cmap->urlprefix . $cmap->accession : NULL;

  $featuremap->num_loci = $num_loci;
  $featuremap->num_lg = $num_lg;
  $featuremap->cmap_url = $cmap_url;

  // this is the preprocess for the genetic map overview form drawn at the top of the Map Overview
  tripal_map_preprocess_tripal_map_genetic_map_overview($variables);
}


function tripal_map_preprocess_tripal_map_genetic_map_overview(&$variables) {
  $node = $variables['node'];
  $featuremap = $variables['node']->featuremap; // duplicate from above

  // if we are editing an existing node then the map is already part of the node
  $featuremap_id = $featuremap->featuremap_id;
  $variables['tripal_map_genetic_map_overview_form'] = drupal_get_form('tripal_map_genetic_map_overview_form', $featuremap_id);

}


function tripal_map_preprocess_mainlab_feature_genetic_marker_map_positions(&$variables) {

  $feature = $variables['node']->feature;
  // get map positions
  $results = chado_query(
    "SELECT CF.nid AS nid, FM.name AS name, FM.featuremap_id as id, X.accession,
    DB.urlprefix, LG.name AS linkage_group, BIN.name AS bin,
    LGP.value AS chr, FPP.value AS locus_start, LOCUS.uniquename AS locus_name

    FROM {feature} LOCUS
    INNER JOIN {feature_relationship} FR 		ON FR.subject_id = LOCUS.feature_id
    INNER JOIN {featurepos} FP 					ON LOCUS.feature_id = FP.feature_id
    INNER JOIN {featuremap} FM 					ON FP.featuremap_id = FM.featuremap_id
    LEFT JOIN {featuremap_dbxref} FD 			ON FP.featuremap_id = FD.featuremap_id
    LEFT JOIN {dbxref} X 						ON FD.dbxref_id = X.dbxref_id
    LEFT JOIN {db}	 							ON db.db_id = X.db_id
    INNER JOIN {feature} LG 					ON FP.map_feature_id = LG.feature_id
    LEFT JOIN
      (SELECT * FROM {featureprop} WHERE type_id =
      (SELECT cvterm_id
        FROM {cvterm}
        WHERE name = 'chr_number'
        AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
      )
    ) LGP ON LG.feature_id = LGP.feature_id
    INNER JOIN {featureposprop} FPP 			ON FP.featurepos_id = FPP.featurepos_id
    LEFT JOIN
      (SELECT F2.name, FR2.subject_id FROM {feature} F2
        INNER JOIN {feature_relationship} FR2 ON FR2.object_id = F2.feature_id
        WHERE FR2.type_id =
        (SELECT cvterm_id FROM {cvterm} WHERE name = 'located_in' AND 
          cv_id = (SELECT cv_id FROM {cv} WHERE name = 'relationship'))
        ) BIN ON LOCUS.feature_id = BIN.subject_id
    LEFT JOIN chado_featuremap CF ON FM.featuremap_id = CF.featuremap_id
    WHERE FR.type_id =
      (SELECT cvterm_id
        FROM {cvterm}
        WHERE name = 'instance_of'
        AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'relationship')
      )
    AND LOCUS.type_id =
      (SELECT cvterm_id
        FROM {cvterm}
        WHERE name = 'marker_locus'
        AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
      )
    AND FPP.type_id =
      (SELECT cvterm_id
        FROM {cvterm}
        WHERE name = 'start'
        AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
      )
    AND FR.object_id = :object_id", array(':object_id' => $feature->feature_id));
  $positions = array();
  $counter = 0;
  while ($pos = $results->fetchObject()) {
    $positions [$counter] = $pos;
    $counter ++;
  }
  $feature->map_positions = $positions;

}


function tripal_map_preprocess_mainlab_feature_QTL_map_positions(&$variables) {

  $feature = $variables['node']->feature;
  $sql = "
  SELECT CF.nid AS nid, FM.name AS name, FM.featuremap_id as id, X.accession,
  DB.urlprefix,  LG.name AS linkage_group, BIN.name AS bin,
  LGP.value AS chr, START.value AS QTL_start, STOP.value AS QTL_stop,
  PEAK.value AS QTL_peak,  QTL.uniquename AS QTL_name
  
  FROM {feature} QTL
  INNER JOIN {featurepos} FP 							ON QTL.feature_id = FP.feature_id
  INNER JOIN {featuremap} FM 							ON FP.featuremap_id = FM.featuremap_id
  LEFT JOIN {featuremap_dbxref} FD 					ON FP.featuremap_id = FD.featuremap_id
  LEFT JOIN {dbxref} X 								ON FD.dbxref_id = X.dbxref_id
  LEFT JOIN {db} 										ON db.db_id = X.db_id
  INNER JOIN {feature} LG 							ON FP.map_feature_id = LG.feature_id
  LEFT JOIN (
    SELECT FP.feature_id, FP.value
    FROM {featureprop} FP
    WHERE FP.type_id =
      (SELECT cvterm_id
      FROM {cvterm}
      WHERE name = 'chr_number' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
      )
  ) LGP ON LG.feature_id = LGP.feature_id
  LEFT JOIN (
    SELECT featurepos_id, value
    FROM {featureposprop} FPP
    WHERE FPP.type_id =
      (SELECT cvterm_id
      FROM {cvterm}
      WHERE name = 'start' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
      )
  ) START                                           ON FP.featurepos_id = START.featurepos_id
  LEFT JOIN (
    SELECT featurepos_id, value
    FROM {featureposprop} FPP
    WHERE FPP.type_id =
      (SELECT cvterm_id
      FROM {cvterm}
      WHERE name = 'stop' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
      )
  ) STOP 										ON FP.featurepos_id = STOP.featurepos_id
  LEFT JOIN (
    SELECT featurepos_id, value
    FROM {featureposprop} FPP
    WHERE FPP.type_id =
      (SELECT cvterm_id
      FROM {cvterm}
      WHERE name = 'qtl_peak' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
      )
  ) PEAK 											ON FP.featurepos_id = PEAK.featurepos_id
  LEFT JOIN (
    SELECT F2.name, FR2.subject_id
    FROM {feature} F2
    INNER JOIN {feature_relationship} FR2 		ON FR2.object_id = F2.feature_id
      WHERE FR2.type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'located_in')
  ) BIN 											ON QTL.feature_id = BIN.subject_id
  LEFT JOIN public.chado_featuremap CF 			ON FM.featuremap_id = CF.featuremap_id
    WHERE QTL.type_id = (SELECT type_id FROM {feature} WHERE feature_id = :feature_id1) AND
      QTL.feature_id = :feature_id2
  ";
  $qtl_map_pos = tripal_map_mainlab_tripal_dbresult_to_array(chado_query($sql, array(':feature_id1' => $feature->feature_id,
    ':feature_id2' => $feature->feature_id)));

  $feature->mainlab_qtl->map_positions = $qtl_map_pos;
}


function tripal_map_preprocess_mainlab_feature_heritable_phenotypic_marker_map_positions(&$variables) {

  $feature = $variables['node']->feature;
  $sql = "
  SELECT CF.nid AS nid, FM.name AS name, FM.featuremap_id as id, X.accession, DB.urlprefix,
  LG.name AS linkage_group, BIN.name AS bin, LGP.value AS chr,
  START.value AS MTL_start, STOP.value AS MTL_stop, PEAK.value AS MTL_peak,
  MTL.uniquename AS MTL_name
  FROM {feature} MTL
  INNER JOIN {featurepos} FP 					ON MTL.feature_id = FP.feature_id
  INNER JOIN {featuremap} FM 					ON FP.featuremap_id = FM.featuremap_id
  LEFT JOIN {featuremap_dbxref} FD 			ON FP.featuremap_id = FD.featuremap_id
  LEFT JOIN {dbxref} X 						ON FD.dbxref_id = X.dbxref_id
  LEFT JOIN {db} 								ON db.db_id = X.db_id
  INNER JOIN {feature} LG 					ON FP.map_feature_id = LG.feature_id
  LEFT JOIN (
    SELECT FP.feature_id, FP.value
    FROM {featureprop} FP
    WHERE FP.type_id =
      (SELECT cvterm_id
      FROM {cvterm}
      WHERE name = 'chr_number' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
  	  )
  ) LGP 										ON LG.feature_id = LGP.feature_id
  LEFT JOIN (
    SELECT featurepos_id, value
      FROM {featureposprop} FPP
      WHERE FPP.type_id = (
        SELECT cvterm_id
        FROM {cvterm}
        WHERE name = 'start' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN'))
  ) START 									ON FP.featurepos_id = START.featurepos_id
  LEFT JOIN (
    SELECT featurepos_id, value
    FROM {featureposprop} FPP
    WHERE FPP.type_id = (
      SELECT cvterm_id
      FROM {cvterm}
      WHERE name = 'stop' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
    )
  ) STOP 										ON FP.featurepos_id = STOP.featurepos_id
  LEFT JOIN (
    SELECT featurepos_id, value
    FROM {featureposprop} FPP
    WHERE FPP.type_id = (
      SELECT cvterm_id
      FROM {cvterm}
      WHERE name = 'mtl_peak' AND cv_id = (SELECT cv_id FROM {cv} WHERE name = 'MAIN')
    )
  ) PEAK 										ON FP.featurepos_id = PEAK.featurepos_id
  LEFT JOIN (
    SELECT F2.name, FR2.subject_id
    FROM {feature} F2
      INNER JOIN {feature_relationship} FR2 	ON FR2.object_id = F2.feature_id
        WHERE FR2.type_id = (
        SELECT cvterm_id FROM {cvterm} WHERE name = 'located_in')
  ) BIN 									ON MTL.feature_id = BIN.subject_id
  LEFT JOIN public.chado_featuremap CF 		ON FM.featuremap_id = CF.featuremap_id
    WHERE MTL.type_id = (SELECT type_id FROM {feature} WHERE feature_id = :feature_id1) AND
    MTL.feature_id = :feature_id2
  ";
  $mtl_map_pos = tripal_map_mainlab_tripal_dbresult_to_array(chado_query($sql, array(':feature_id1' => $feature->feature_id,
  ':feature_id2' => $feature->feature_id)));

  $feature->mainlab_mtl->map_positions = $mtl_map_pos;
}


// Convert a database result set into an object array
function tripal_map_mainlab_tripal_dbresult_to_array($dbresult) {
  $arr = array();
  while ($obj = $dbresult->fetchObject()) {
    array_push($arr, $obj);
  }
  
  return $arr;
}

// Link to Tripal v2 node. If node is not available, link to Tripal v3 entity
function tripal_map_mainlab_tripal_link_record ($base_table, $record_id) {
	$link = NULL;
	
	// tripal v2 link (node)
	if (function_exists('chado_get_nid_from_id')) {
	    $nid = chado_get_nid_from_id($base_table, $record_id);
	    if ($nid) {
	        $link = "/node/$nid";
	    }
	}
	
	// tripal v3 link (entity)
	if (function_exists('tripal_get_chado_entity_id')) {
		$entity_id = tripal_get_chado_entity_id ($base_table, $record_id);
		if ($entity_id) {
			$link = "/bio_data/$entity_id";
		}
	}
	return $link;
}

