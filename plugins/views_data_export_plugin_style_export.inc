<?php
// $Id$
/**
 * @file
 * Plugin include file for export style plugin.
 */

/**
 * Generalized style plugin for export plugins.
 *
 * @ingroup views_style_plugins
 */
class views_data_export_plugin_style_export extends views_plugin_style {

  /**
   * Set options fields and default values.
   *
   * @return
   * An array of options information.
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['attach_text'] = array(
      'default' => $this->definition['export feed text'],
      'translatable' => TRUE,
    );
    $options['provide_file'] = array(
      'default' => FALSE,
      'translatable' => FALSE,
    );
    $options['filename'] = array(
      'default' => $this->definition['export feed file'],
      'translatable' => FALSE,
    );
    $options['parent_sort'] = array(
      'default' => FALSE,
      'translatable' => FALSE,
    );
    return $options;
  }

  /**
   * Options form mini callback.
   *
   * @param $form
   * Form array to add additional fields to.
   * @param $form_state
   * State of the form.
   * @return
   * None.
   */
  function options_form(&$form, &$form_state) {
    $form['attach_text'] = array(
      '#type' => 'textfield',
      '#title' => t('Attach text'),
      '#default_value' => $this->options['attach_text'],
      '#description' => t('This text is used in building the feed link. By default it is the "alt" text for the feed image.'),
    );
    $form['provide_file'] = array(
      '#type' => 'checkbox',
      '#title' => t('Provide as file'),
      '#default_value' => $this->options['provide_file'],
      '#description' => t('By deselecting this, the xml file will be provided as a feed instead of a file for download.'),
    );
    $form['filename'] = array(
      '#type' => 'textfield',
      '#title' => t('Filename'),
      '#default_value' => $this->options['filename'],
      '#description' => t('The filename that will be suggested to the browser for downloading purposes. %view will be replaced with the view name.'),
      '#process' => array('ctools_dependent_process'),
      '#dependency' => array(
        'edit-style-options-provide-file' => array(TRUE),
      ),
    );
    $form['parent_sort'] = array(
      '#type' => 'checkbox',
      '#title' => t('Parent sort'),
      '#default_value' => $this->options['parent_sort'],
      '#description' => t('Try to apply any additional sorting from the attached display like table sorting to the exported feed.'),
    );
  }

  /**
   * Attach this view to another display as a feed.
   *
   * Provide basic functionality for all export style views like attaching a
   * feed image link.
   */
  function attach_to($display_id, $path, $title) {

    $type = $this->definition['export feed type'];
    $theme_pattern = array(
      'views_data_export_feed_icon__' . $this->view->name . '__' . $display_id . '__' . $type,
      'views_data_export_feed_icon__' . $this->view->name . '__' . $display_id,
      'views_data_export_feed_icon__' . $this->view->name . '__' . $type,
      'views_data_export_feed_icon__' . $display_id . '__' . $type,
      'views_data_export_feed_icon__' . $display_id,
      'views_data_export_feed_icon__' . $type,
      'views_data_export_feed_icon',
    );
    $query = $this->view->get_exposed_input();
    // Stash the display id we're coming form in the url so we can hijack it later.
    if ($this->options['parent_sort']) {
      $query['attach'] = $display_id;
    }
    $this->view->feed_icon .= theme($theme_pattern, array(
        'image_path' => $this->defintion['export feed icon'],
        'url' => $this->view->get_url(NULL, $path),
        'query' => $query,
        'text' => $this->options['attach_text'],
      )
    );
  }

  function build_sort() {

    // Bypass doing any sort of testing if parent sorting is disabled.
    if (!$this->options['parent_sort']) {
      return parent::build_sort();
    }

    $displays = $this->display->handler->get_option('displays');

    // Here is later. We can get the passed argument and use it to know which
    // display we can from and then do some addition processing.
    // If the display exists and is attached these two tests will succeed.
    if (isset($_GET['attach']) && isset($displays[$_GET['attach']]) && $displays[$_GET['attach']]) {
      // Setup the second style we're going to be using to sort on.
      $plugin_id = $displays[$_GET['attach']];
      $parent_display = $this->view->display[$plugin_id];
      $style_name = $parent_display->handler->get_option('style_plugin');
      $style_options = $parent_display->handler->get_option('style_options');
      $this->extra_style = views_get_plugin('style', $style_name);
      $this->extra_style->init($this->view, $parent_display, $style_options);

      // Call the second styles sort funciton and return the value.
      return $this->extra_style->build_sort();
    }
  }

  function build_sort_post() {
    // If we found an extra style plugin earlier, pass off the build_sort_post call to it.
    if (isset($this->extra_style)) {
      return $this->extra_style->build_sort_post();
    }
    else {
      return parent::build_sort_post();
    }
  }

  /**
   * Render the display in this style.
   */
  function render() {
    if ($this->uses_row_plugin() && empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }

    $output = '';
    $rows['header'] = $this->render_header();
    $rows['body'] = $this->render_body();
    $rows['footer'] = $this->render_footer();
    $title = '';
    $output .= theme($this->theme_functions(), array('view' => $this->view, 'options' => $this->options, 'rows' => $rows, 'title' => $title));
    return $output;
  }

  function render_header() {
    $rows = array();
    $title = '';
    $output = '';
    $output .= theme($this->definition['additional themes base'] . '_header', array('view' => $this->view, 'options' => $this->options, 'rows' => $rows, 'title' => $title));

    return $output;
  }

  function render_footer() {
    $rows = array();
    $title = '';
    $output = '';
    $output .= theme($this->definition['additional themes base'] . '_footer', array('view' => $this->view, 'options' => $this->options, 'rows' => $rows, 'title' => $title));

    return $output;
  }

  function render_body() {
    if ($this->uses_row_plugin() && empty($this->row_plugin)) {
      vpr('views_plugin_style_default: Missing row plugin');
      return;
    }

    // Group the rows according to the grouping field, if specified.
    $sets = $this->render_grouping($this->view->result, $this->options['grouping']);

    // Render each group separately and concatenate.  Plugins may override this
    // method if they wish some other way of handling grouping.
    $output = '';
    foreach ($sets as $title => $records) {
      if ($this->uses_row_plugin()) {
        $rows = array();
        foreach ($records as $row_index => $row) {
          $this->view->row_index = $row_index;
          $rows[] = $this->row_plugin->render($row);
        }
      }
      else {
        $rows = $records;
      }

      $title = '';
      $output .= theme($this->definition['additional themes base'] . '_body', array('view' => $this->view, 'options' => $this->options, 'rows' => $rows, 'title' => $title));
    }
    unset($this->view->row_index);
    return $output;

  }

  /**
   * Provide a full list of possible theme templates used by this style.
   */
  function theme_functions($hook = NULL) {
    if (is_null($hook)) {
      $hook = $this->definition['theme'];
    }
    return views_theme_functions($hook, $this->view, $this->display);
  }

  /**
   * Add any HTTP headers that this style plugin wants to.
   */
  function add_http_headers() {

    drupal_add_http_header('Cache-Control', 'max-age=60, must-revalidate');

    if (!empty($this->definition['export headers'])) {
      foreach ($this->definition['export headers'] as $name => $value) {
        drupal_add_http_header($name, $value);
      }
    }

    if (isset($this->options['filename']) && !empty($this->options['provide_file'])) {
      $filename = strtr(
        $this->options['filename'],
        array('%view' => check_plain($this->view->name))
      );

      if ($filename) {
        drupal_add_http_header('Content-Disposition', 'attachment; filename="'. $filename .'"');
      }
    }
  }
}
