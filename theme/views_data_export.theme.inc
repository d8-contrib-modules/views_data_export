<?php
// $Id$

/**
 * @file
 * Theme related functions for processing our output style plugins.
 *
 * Views bug: http://drupal.org/node/593336
 */

/**
 * Theme a feed link.
 *
 * This theme function uses the theme pattern system to allow it to be
 * overidden in a more specific manner. The options for overiding this include
 * providing per display id; per type; per display id and per type.
 *
 * e.g.
 * For the view "export_test" with the display "page_1" and the type "csv" you
 * would have the following options.
 *   views_data_export_feed_icon__export_test__page_1__csv
 *   views_data_export_feed_icon__export_test__page_1
 *   views_data_export_feed_icon__export_test__csv
 *   views_data_export_feed_icon__page_1__csv
 *   views_data_export_feed_icon__page_1
 *   views_data_export_feed_icon__csv
 *   views_data_export_feed_icon
 *
 * @ingroup themeable
 */
function theme_views_data_export_feed_icon($image_path, $url, $query = '', $text = '') {
  $url_options = array('html' => true);
  if ($query) {
    $url_options['query'] = $query;
  }
  $image = theme('image', $image_path, $text, $text);
  return l($image, $url, $url_options);
}

/**
 * Theme callback for the export complete page.
 *
 * @param $file
 *  Link to output file
 */
function theme_views_data_export_complete_page($file, $errors = array(), $return_url = '') {
  drupal_set_title(t('Data export successful'));
  drupal_set_html_head('<meta http-equiv="refresh" content="3;url='. check_plain($file) . '" />');
  $output = '';
  $output .= '<p>';
  $output .= t('Your export has been created. View/download the file <a href="@link">here</a> (will automatically download in 3 seconds.)', array('@link' => $file));
  $output .= '</p>';

  if (!empty($return_url)) {
    $output .= '<p>';
    $output .= l(t('Return to previous page'), $return_url);
    $output .= '</p>';
  }
  return $output;
}


function template_preprocess_views_data_export(&$vars) {
  $vars['header'] = $vars['rows']['header'];
  $vars['body'] = $vars['rows']['body'];
  $vars['footer'] = $vars['rows']['footer'];

  $view     = $vars['view'];
  $fields   = &$view->field;
}

function template_preprocess_views_data_export_csv_header(&$vars) {
  _views_data_export_header_shared_preprocess($vars);

  // Make sure we catch saved options that are misspelled. LEGACY
  if (isset($vars['options']['seperator'])) {
    $vars['options']['separator'] = $vars['options']['seperator'];
  }
  // Support old misspelled templates. LEGACY
  $vars['seperator'] =
    $vars['separator'] = $vars['options']['separator'];

  // Special handling when quoted values are involved.
  if ($vars['options']['quote']) {
    $wrap = '"';
    $replace_value = '""';
  }
  else {
    $wrap = '';
    $replace_value = '';
  }

  // Format header values.
  foreach ($vars['header'] as $key => $value) {
    $output = decode_entities(strip_tags($value));
    if ($vars['options']['trim']) {
      $output = trim($output);
    }
    $vars['header'][$key] = $wrap . str_replace('"', $replace_value, $output) . $wrap;
  }
}

function template_preprocess_views_data_export_csv_body(&$vars) {
  _views_data_export_body_shared_preprocess($vars);

  // Make sure we catch saved options that are misspelled. LEGACY
  if (isset($vars['options']['seperator'])) {
    $vars['options']['separator'] = $vars['options']['seperator'];
  }
  // Support old misspelled templates. LEGACY
  $vars['seperator'] =
    $vars['separator'] = $vars['options']['separator'];

  // Special handling when quoted values are involved.
  if ($vars['options']['quote']) {
    $wrap = '"';
    $replace_value = '""';
  }
  else {
    $wrap = '';
    $replace_value = '';
  }

  // Format row values.
  foreach ($vars['themed_rows'] as $i => $values) {
    foreach ($values as $j => $value) {
      $output = decode_entities(strip_tags($value));
      if ($vars['options']['trim']) {
        $output = trim($output);
      }
      $vars['themed_rows'][$i][$j] = $wrap . str_replace('"', $replace_value, $output) . $wrap;
    }
  }
}

/**
 * Preprocess csv output template.
 */
function template_preprocess_views_data_export_csv(&$vars) {
  // TODO Replace items with themed_rows.
  _views_data_export_shared_preprocess($vars);

  // Make sure we catch saved options that are misspelled. LEGACY
  if (isset($vars['options']['separator'])) {
    $vars['options']['separator'] = $vars['options']['seperator'];
  }
  // Support old misspelled templates. LEGACY
  $vars['seperator'] =
    $vars['separator'] = $vars['options']['separator'];

  // Special handling when quoted values are involved.
  if ($vars['options']['quote']) {
    $wrap = '"';
    $replace_value = '""';
  }
  else {
    $wrap = '';
    $replace_value = '';
  }

  // Format header values.
  foreach ($vars['header'] as $key => $value) {
    $output = decode_entities(strip_tags($value));
    if ($vars['options']['trim']) {
      $output = trim($output);
    }
    $vars['header'][$key] = $wrap . str_replace('"', $replace_value, $output) . $wrap;
  }

  // Format row values.
  foreach ($vars['themed_rows'] as $i => $values) {
    foreach ($values as $j => $value) {
      $output = decode_entities(strip_tags($value));
      if ($vars['options']['trim']) {
        $output = trim($output);
      }
      $vars['themed_rows'][$i][$j] = $wrap . str_replace('"', $replace_value, $output) . $wrap;
    }
  }
}

/**
 * Preprocess txt output template.
 */
function template_preprocess_views_data_export_txt_body(&$vars) {
  _views_data_export_header_shared_preprocess($vars);
  _views_data_export_body_shared_preprocess($vars);
}

function template_preprocess_views_data_export_doc_body(&$vars) {
  // Pass through the generic MS Office preprocess.
  template_preprocess_views_data_export_msoffice_body($vars);
}

function template_preprocess_views_data_export_xls_body(&$vars) {
  // Pass through the generic MS Office preprocess.
  template_preprocess_views_data_export_msoffice_body($vars);
}

function template_preprocess_views_data_export_msoffice_body(&$vars) {
  _views_data_export_header_shared_preprocess($vars);
  _views_data_export_body_shared_preprocess($vars);

  // Construct the tbody of a table, see theme_table().

  $ts = tablesort_init($vars['header']);

  $flip = array(
    'even' => 'odd',
    'odd' => 'even',
  );
  $class = 'even';
  foreach ($vars['themed_rows'] as $number => $row) {
    $attributes = array();

    // Check if we're dealing with a simple or complex row
    if (isset($row['data'])) {
      foreach ($row as $key => $value) {
        if ($key == 'data') {
          $cells = $value;
        }
        else {
          $attributes[$key] = $value;
        }
      }
    }
    else {
      $cells = $row;
    }
    if (count($cells)) {
      // Add odd/even class
      $class = $flip[$class];
      if (isset($attributes['class'])) {
        $attributes['class'] .= ' ' . $class;
      }
      else {
        $attributes['class'] = $class;
      }

      // Build row
      $output .= ' <tr' . drupal_attributes($attributes) . '>';
      $i = 0;
      foreach ($cells as $cell) {
        $cell = tablesort_cell($cell, $header, $ts, $i++);
        $output .= _theme_table_cell($cell);
      }
      $output .= " </tr>\n";
    }
  }


  $vars['tbody'] = preg_replace('/<\/?(a|span) ?.*?>/', '', $output); // strip 'a' and 'span' tags

}

function template_preprocess_views_data_export_doc_header(&$vars) {
  // Pass through the generic MS Office preprocess.
  template_preprocess_views_data_export_msoffice_header($vars);
}

function template_preprocess_views_data_export_xls_header(&$vars) {
  // Pass through the generic MS Office preprocess.
  template_preprocess_views_data_export_msoffice_header($vars);
}

function template_preprocess_views_data_export_msoffice_header(&$vars) {
  _views_data_export_header_shared_preprocess($vars);

  // Need to do a little work to construct the table header, see theme_table().
  $vars['header_row'] = '';
  $vars['header_row'] .= '<thead><tr>';

  $ts = tablesort_init($vars['header']);

  foreach ($vars['header'] as $cell) {
    $cell = tablesort_header($cell, $header, $ts);
    $vars['header_row'] .= _theme_table_cell($cell, TRUE);
  }

  $vars['header_row'] .= '</tr></thead>';

  $vars['header_row'] = preg_replace('/<\/?(a|span) ?.*?>/', '', $vars['header_row']); // strip 'a' and 'span' tags
}

/**
 * Preprocess xml output template.
 */
function template_preprocess_views_data_export_xml_body(&$vars) {
  _views_data_export_header_shared_preprocess($vars);
  _views_data_export_body_shared_preprocess($vars);

  foreach ($vars['themed_rows'] as $num => $row) {
    foreach ($row as $field => $content) {
      // Prevent double encoding of the ampersand. Look for the entities produced by check_plain().
      $content = preg_replace('/&(?!(amp|quot|#039|lt|gt);)/', '&amp;', $content);
      // Convert < and > to HTML entities.
      $content = str_replace(
        array('<', '>'),
        array('&lt;', '&gt;'),
        $content);
      $vars['themed_rows'][$num][$field] = $content;
    }
  }

  foreach ($vars['header'] as $field => $header) {
    if ($vars['options']['transform']) {
      switch ($vars['options']['transform_type']) {
        case 'dash':
          $vars['xml_tag'][$field] = str_replace(' ', '-', $header);
          break;
        case 'underline':
          $vars['xml_tag'][$field] = str_replace(' ', '_', $header);
          break;
        case 'camel':
          $vars['xml_tag'][$field] = str_replace(' ', '', ucwords($header));
          $vars['xml_tag'][$field][0] = strtolower($vars['xml_tag'][$field][0]);
          break;
        case 'pascal':
          $vars['xml_tag'][$field] = str_replace(' ', '', ucwords($header));
          break;
      }
    }
  }
}

/**
 * Shared helper function for export preprocess functions.
 */
function _views_data_export_header_shared_preprocess(&$vars) {
  $view     = $vars['view'];
  $fields   = &$view->field;

  $vars['header'] = array();
  foreach ($fields as $key => $field) {
    if (empty($field->options['exclude'])) {
      $vars['header'][$key] = check_plain($field->label());
    }
  }

}

/**
 * Shared helper function for export preprocess functions.
 */
function _views_data_export_body_shared_preprocess(&$vars) {
  $view     = $vars['view'];
  $fields   = &$view->field;

  $rows = $vars['rows'];

  $vars['themed_rows'] = array();
  $keys = array_keys($fields);
  foreach ($rows as $num => $row) {
    $vars['themed_rows'][$num] = array();

    foreach ($keys as $id) {
      if (empty($fields[$id]->options['exclude'])) {
        $vars['themed_rows'][$num][$id] = $view->style_plugin->rendered_fields[$num][$id];
      }
    }
  }
}
