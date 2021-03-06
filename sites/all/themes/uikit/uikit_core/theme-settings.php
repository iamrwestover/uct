<?php

/**
 * @file
 * Provides theme settings for UIkit themes.
 */

/**
 * Implements hook_form_system_theme_settings_alter().
 */
function uikit_form_system_theme_settings_alter(&$form, &$form_state, $form_id = NULL) {
  global $theme_key;

  // General "alters" use a form id. Settings should not be set here. The only
  // thing useful about this is if you need to alter the form for the running
  // theme and *not* the theme setting.
  // @see http://drupal.org/node/943212
  if (isset($form_id)) {
    return;
  }

  // Get the active theme name.
  $theme_key = $form_state['build_info']['args'][0] === $theme_key ? $form_state['build_info']['args'][0] : $theme_key;

  // Build the markup for the layout demos.
  $demo_layout = '<div class="uk-layout-wrapper">';
  $demo_layout .= '<div class="uk-layout-container">';
  $demo_layout .= '<div class="uk-layout-content"></div>';
  $demo_layout .= '<div class="uk-layout-sidebar uk-layout-sidebar-left"></div>';
  $demo_layout .= '<div class="uk-layout-sidebar uk-layout-sidebar-right"></div>';
  $demo_layout .= '</div></div>';

  // Get the sidebar positions for each layout.
  $standard_sidebar_pos = theme_get_setting('standard_sidebar_positions', $theme_key);
  $tablet_sidebar_pos = theme_get_setting('tablet_sidebar_positions', $theme_key);
  $mobile_sidebar_pos = theme_get_setting('mobile_sidebar_positions', $theme_key);

  // Get all menus.
  $menus = menu_get_menus();

  // Get the main and secondary menus.
  $main_menu = variable_get('menu_main_links_source', 'main-menu');
  $secondary_menu = variable_get('menu_secondary_links_source', 'user-menu');

  // Set the charset options.
  $charsets = array(
    'utf-8' => 'UTF-8: All languages (Recommended)',
    'ISO-8859-1' => 'ISO 8859-1: Latin 1',
    'ISO-8859-2' => 'ISO 8859-2: Central & East European',
    'ISO-8859-3' => 'ISO 8859-3: South European, Maltese & Esperanto',
    'ISO-8859-4' => 'ISO 8859-4: North European',
    'ISO-8859-5' => 'ISO 8859-5: Cyrillic',
    'ISO-8859-6' => 'ISO 8859-6: Arabic',
    'ISO-8859-7' => 'ISO 8859-7: Modern Greek',
    'ISO-8859-8' => 'ISO 8859-8: Hebrew & Yiddish',
    'ISO-8859-9' => 'ISO 8859-9: Turkish',
    'ISO-8859-10' => 'ISO 8859-10: Nordic (Lappish, Inuit, Icelandic)',
    'ISO-8859-11' => 'ISO 8859-11: Thai',
    'ISO-8859-13' => 'ISO 8859-13: Baltic Rim',
    'ISO-8859-14' => 'ISO 8859-14: Celtic',
    'ISO-8859-16' => 'ISO 8859-16: South-Eastern Europe',
  );

  // Set the x-ua-compatible options.
  $x_ua_compatible_ie_options = array(
    0 => 'None (Recommended)',
    'edge' => 'Highest supported document mode',
    '5' => 'Quirks Mode',
    '7' => 'IE7 mode',
    '8' => 'IE8 mode',
    '9' => 'IE9 mode',
    '10' => 'IE10 mode',
    '11' => 'IE11 mode',
  );

  // Set the navbar margin options.
  $navbar_margin_top_options = array(
    'No top margin',
    'Normal top margin',
    'Smaller top margin',
    'Larger top margin',
  );
  $navbar_margin_bottom_options = array(
    'No bottom margin',
    'Normal bottom margin',
    'Smaller bottom margin',
    'Larger bottom margin',
  );

  // Build the markup for the local task demos.
  $demo_local_tasks = '<ul class="uk-subnav">';
  $demo_local_tasks .= '<li class="uk-active"><a href="#">Item</a></li>';
  $demo_local_tasks .= '<li><a href="#">Item</a></li>';
  $demo_local_tasks .= '<li><a href="#">Item</a></li>';
  $demo_local_tasks .= '<li class="uk-disabled"><a href="#">Disabled</a></li>';
  $demo_local_tasks .= '</ul>';

  // Set the subnav options.
  $subnav_options = array(
    0 => 'Basic subnav',
    'uk-subnav-line' => 'Subnav line',
    'uk-subnav-pill' => 'Subnav pill',
  );

  // Set the region style options.
  $region_style_options = array(
    0 => 'No style',
    'panel' => 'Panel',
    'block' => 'Block',
  );

  $viewport_scale = array(
    -1 => t('-- Select --'),
    '0' => '0',
    '1' => '1',
    '2' => '2',
    '3' => '3',
    '4' => '4',
    '5' => '5',
    '6' => '6',
    '7' => '7',
    '8' => '8',
    '9' => '9',
    '10' => '10',
  );

  // Fetch a list of regions for the current theme.
  $all_regions = system_region_list($theme_key);

  // Create vertical tabs for all UIkit related settings.
  $form['uikit'] = array(
    '#type' => 'vertical_tabs',
    '#attached' => array(
      'css' => array(
        drupal_get_path('theme', 'uikit') . '/css/uikit.admin.css',
      ),
      'js' => array(drupal_get_path('theme', 'uikit') . '/js/uikit.admin.js'),
    ),
    '#prefix' => '<h3>' . t('UIkit Settings') . '</h3>',
    '#weight' => -10,
  );

  // UIkit theme styles.
  $form['theme'] = array(
    '#type' => 'fieldset',
    '#title' => t('Theme styles'),
    '#description' => t('UIkit comes with a basic theme and two neat themes to get you started. Here you can select which base style to start with.'),
    '#group' => 'uikit',
    '#attributes' => array(
      'class' => array(
        'uikit-theme-settings-form',
      ),
    ),
  );
  $form['theme']['base_style'] = array(
    '#type' => 'select',
    '#title' => t('Base style'),
    '#options' => array(
      0 => t('UIkit default'),
      'almost-flat' => t('UIkit almost flat'),
      'gradient' => t('UIkit gradient'),
    ),
    '#description' => t('Select which base style to use.<ol><li><strong>UIkit default:</strong> No border radius or gradients</li><li><strong>UIkit almost flat:</strong> Small border and border radius</li><li><strong>UIkit gradient:</strong> Almost flat style with gradient backgrounds.</li></ol>'),
    '#default_value' => theme_get_setting('base_style', $theme_key),
  );

  // Mobile settings.
  $form['mobile_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mobile settings'),
    '#description' => t("Adjust the mobile layout settings to enhance your users' experience on smaller devices."),
    '#group' => 'uikit',
    '#attributes' => array(
      'class' => array(
        'uikit-mobile-settings-form',
      ),
    ),
  );
  $form['mobile_settings']['mobile_metadata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mobile metadata'),
    '#description' => t('HTML5 has attributes that can be defined in meta elements. Here you can control some of these attributes.'),
  );
  $form['mobile_settings']['mobile_metadata']['x_ua_compatible'] = array(
    '#type' => 'select',
    '#title' => t('<code>x_ua_compatible</code> IE Mode'),
    '#options' => $x_ua_compatible_ie_options,
    '#default_value' => theme_get_setting('x_ua_compatible', $theme_key),
    '#description' => t('In some cases, it might be necessary to restrict a webpage to a document mode supported by an older version of Windows Internet Explorer. Here we look at the x-ua-compatible header, which allows a webpage to be displayed as if it were viewed by an earlier version of the browser. @see !legacy', array(
      '!legacy' => '<a href="https://msdn.microsoft.com/en-us/library/jj676915(v=vs.85).aspx" target="_blank">' . t('Specifying legacy document modes') . '</a>',
    )),
  );
  $form['mobile_settings']['mobile_metadata']['meta_charset'] = array(
    '#type' => 'select',
    '#title' => t('<code>charset</code>'),
    '#options' => $charsets,
    '#description' => t('Specify the character encoding for the HTML document.'),
    '#default_option' => theme_get_setting('meta_charset', $theme_key),
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport'] = array(
    '#type' => 'fieldset',
    '#title' => t('Viewport metadata'),
    '#description' => t('Gives hints about the size of the initial size of the viewport. This pragma is used by several mobile devices only.'),
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['device_width'] = array(
    '#type' => 'fieldset',
    '#title' => t('Width'),
    '#collapsible' => TRUE,
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['device_width']['viewport_device_width_ratio'] = array(
    '#type' => 'select',
    '#title' => t('Device width ratio'),
    '#description' => t('Defines the ratio between the device width (device-width in portrait mode or device-height in landscape mode) and the viewport size. Literal device width is defined as <code>device-width</code> and is the recommended value. You can also specify a pixel width by selecting <b>Other</b>, such as <code>300</code>.'),
    '#default_value' => theme_get_setting('viewport_device_width_ratio', $theme_key),
    '#options' => array(
      0 => t('-- Select --'),
      'device-width' => t('Literal device width (Recommended)'),
      '1' => t('Other'),
    ),
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['device_width']['viewport_custom_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom device width'),
    '#description' => t('Defines the width, in pixels, of the viewport. Do not add <b>px</b>, the value must be a non-decimal integer number.'),
    '#default_value' => theme_get_setting('viewport_custom_width', $theme_key),
    '#attributes' => array(
      'size' => 15,
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="viewport_device_width_ratio"]' => array('value' => '1'),
      ),
      'required' => array(
        ':input[name="viewport_device_width_ratio"]' => array('value' => '1'),
      ),
    ),
    '#element_validate' => array('_uikit_viewport_custom_width_validate'),
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['device_height'] = array(
    '#type' => 'fieldset',
    '#title' => t('Height'),
    '#collapsible' => TRUE,
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['device_height']['viewport_device_height_ratio'] = array(
    '#type' => 'select',
    '#title' => t('Device height ratio'),
    '#description' => t('Defines the ratio between the device height (device-height in portrait mode or device-width in landscape mode) and the viewport size. Literal device height is defined as <code>device-height</code> and is the recommended value. You can also specify a pixel height by selecting <b>Other</b>, such as <code>300</code>.'),
    '#default_value' => theme_get_setting('viewport_device_height_ratio', $theme_key),
    '#options' => array(
      0 => t('-- Select --'),
      'device-height' => t('Literal device height (Recommended)'),
      '1' => t('Other'),
    ),
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['device_height']['viewport_custom_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Custom device height'),
    '#description' => t('Defines the height, in pixels, of the viewport. Do not add <b>px</b>, the value must be a non-decimal integer number.'),
    '#default_value' => theme_get_setting('viewport_custom_height', $theme_key),
    '#attributes' => array(
      'size' => 15,
    ),
    '#states' => array(
      'visible' => array(
        ':input[name="viewport_device_height_ratio"]' => array('value' => '1'),
      ),
      'required' => array(
        ':input[name="viewport_device_height_ratio"]' => array('value' => '1'),
      ),
    ),
    '#element_validate' => array('_uikit_viewport_custom_height_validate'),
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['viewport_initial_scale'] = array(
    '#type' => 'select',
    '#title' => t('initial-scale'),
    '#description' => t('Defines the ratio between the device width (device-width in portrait mode or device-height in landscape mode) and the viewport size.'),
    '#default_value' => theme_get_setting('viewport_initial_scale', $theme_key),
    '#options' => $viewport_scale,
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['viewport_maximum_scale'] = array(
    '#type' => 'select',
    '#title' => t('maximum-scale'),
    '#description' => t('Defines the maximum value of the zoom; it must be greater or equal to the minimum-scale or the behavior is indeterminate.'),
    '#default_value' => theme_get_setting('viewport_maximum_scale', $theme_key),
    '#options' => $viewport_scale,
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['viewport_minimum_scale'] = array(
    '#type' => 'select',
    '#title' => t('minimum-scale'),
    '#description' => t('Defines the minimum value of the zoom; it must be smaller or equal to the maximum-scale or the behavior is indeterminate.'),
    '#default_value' => theme_get_setting('viewport_minimum_scale', $theme_key),
    '#options' => $viewport_scale,
  );
  $form['mobile_settings']['mobile_metadata']['meta_viewport']['viewport_user_scalable'] = array(
    '#type' => 'select',
    '#title' => t('user-scalable'),
    '#description' => t('If set to no, the user is not able to zoom in the webpage. Default value is <b>Yes</b>.'),
    '#default_value' => theme_get_setting('viewport_user_scalable', $theme_key),
    '#options' => array(
      1 => t('Yes'),
      0 => t('No'),
    ),
  );

  // Layout settings.
  $form['layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Layout'),
    '#description' => t('Apply our fully responsive fluid grid system and panels, common layout parts like blog articles and comments and useful utility classes.'),
    '#group' => 'uikit',
  );
  $form['layout']['page_layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page Layout'),
    '#description' => t('Change page layout settings.'),
  );
  $form['layout']['page_layout']['standard_layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Standard Layout'),
    '#description' => t('Change layout settings for desktops and large screens.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['layout']['page_layout']['standard_layout']['standard_layout_demo'] = array(
    '#type' => 'container',
  );
  $form['layout']['page_layout']['standard_layout']['standard_layout_demo']['#attributes']['class'][] = 'uk-admin-demo';
  $form['layout']['page_layout']['standard_layout']['standard_layout_demo']['#attributes']['class'][] = 'uk-layout-' . $standard_sidebar_pos;
  $form['layout']['page_layout']['standard_layout']['standard_layout_demo']['standard_demo'] = array(
    '#markup' => '<div id="standard-layout-demo">' . $demo_layout . '</div>',
  );
  $form['layout']['page_layout']['standard_layout']['standard_sidebar_positions'] = array(
    '#type' => 'radios',
    '#title' => t('Sidebar positions'),
    '#description' => t('Position the sidebars in the standard layout.'),
    '#default_value' => theme_get_setting('standard_sidebar_positions', $theme_key),
    '#options' => array(
      'holy-grail' => t('Holy grail'),
      'sidebars-left' => t('Both sidebars left'),
      'sidebars-right' => t('Both sidebars right'),
    ),
  );
  $form['layout']['page_layout']['tablet_layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Tablet Layout'),
    '#description' => t('Change layout settings for tablets and medium screens.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['layout']['page_layout']['tablet_layout']['tablet_layout_demo'] = array(
    '#type' => 'container',
  );
  $form['layout']['page_layout']['tablet_layout']['tablet_layout_demo']['#attributes']['class'][] = 'uk-admin-demo';
  $form['layout']['page_layout']['tablet_layout']['tablet_layout_demo']['#attributes']['class'][] = 'uk-layout-' . $tablet_sidebar_pos;
  $form['layout']['page_layout']['tablet_layout']['tablet_layout_demo']['tablet_demo'] = array(
    '#markup' => '<div id="tablet-layout-demo">' . $demo_layout . '</div>',
  );
  $form['layout']['page_layout']['tablet_layout']['tablet_sidebar_positions'] = array(
    '#type' => 'radios',
    '#title' => t('Sidebar positions'),
    '#description' => t('Position the sidebars in the tablet layout.'),
    '#default_value' => theme_get_setting('tablet_sidebar_positions', $theme_key),
    '#options' => array(
      'holy-grail' => t('Holy grail'),
      'sidebars-left' => t('Both sidebars left'),
      'sidebar-left-stacked' => t('Left sidebar stacked'),
      'sidebars-right' => t('Both sidebars right'),
      'sidebar-right-stacked' => t('Right sidebar stacked'),
    ),
  );
  $form['layout']['page_layout']['mobile_layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mobile Layout'),
    '#description' => t('Change layout settings for mobile devices and small screens.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['layout']['page_layout']['mobile_layout']['mobile_layout_demo'] = array(
    '#type' => 'container',
  );
  $form['layout']['page_layout']['mobile_layout']['mobile_layout_demo']['#attributes']['class'][] = 'uk-admin-demo';
  $form['layout']['page_layout']['mobile_layout']['mobile_layout_demo']['#attributes']['class'][] = 'uk-layout-' . $mobile_sidebar_pos;
  $form['layout']['page_layout']['mobile_layout']['mobile_layout_demo']['mobile_demo'] = array(
    '#markup' => '<div id="mobile-layout-demo">' . $demo_layout . '</div>',
  );
  $form['layout']['page_layout']['mobile_layout']['mobile_sidebar_positions'] = array(
    '#type' => 'radios',
    '#title' => t('Sidebar positions'),
    '#description' => t('Position the sidebars in the mobile layout.'),
    '#default_value' => theme_get_setting('mobile_sidebar_positions', $theme_key),
    '#options' => array(
      'sidebars-stacked' => t('Sidebars stacked'),
      'sidebars-vertical' => t('Sidebars vertical'),
    ),
  );
  $form['layout']['page_layout']['page_container'] = array(
    '#type' => 'checkbox',
    '#title' => t('Page Container'),
    '#description' => t('Add the .uk-container class to the page container to give it a max-width and wrap the main content of your website. For large screens it applies a different max-width.'),
    '#default_value' => theme_get_setting('page_container', $theme_key),
  );
  $form['layout']['page_layout']['page_centering'] = array(
    '#type' => 'checkbox',
    '#title' => t('Page Centering'),
    '#description' => t('To center the page container, use the .uk-container-center class.'),
    '#default_value' => theme_get_setting('page_centering', $theme_key),
  );
  $form['layout']['page_layout']['page_margin'] = array(
    '#type' => 'select',
    '#title' => t('Page margin'),
    '#description' => t('Select the margin to add to the top and bottom of the page container. This is useful, for example, when using the gradient style with a centered page container and a navbar.'),
    '#default_value' => theme_get_setting('page_margin', $theme_key),
    '#options' => array(
      0 => t('No margin'),
      'uk-margin-top' => t('Top margin'),
      'uk-margin-bottom' => t('Bottom margin'),
      'uk-margin' => t('Top and bottom margin'),
    ),
  );
  $form['layout']['region_layout'] = array(
    '#type' => 'fieldset',
    '#title' => t('Region Layout'),
    '#description' => t('Change region layout settings.<br><br>Use the following links to see an example of each component style.<ul class="links"><li><a href="http://getuikit.com/docs/panel.html" target="_blank">Panel</a></li><li><a href="http://getuikit.com/docs/block.html" target="_blank">Block</a></li></ul>'),
  );

  // Load all regions to assign separate settings for each region.
  foreach ($all_regions as $region_key => $region) {
    $form['layout']['region_layout'][$region_key] = array(
      '#type' => 'fieldset',
      '#title' => t('@region region', array('@region' => $region)),
      '#description' => t('Change the @region region settings.', array('@region' => $region)),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['layout']['region_layout'][$region_key][$region_key . '_style'] = array(
      '#type' => 'select',
      '#title' => t('@title style', array('@title' => $region)),
      '#description' => t('Set the style for the @region region. The theme will automatically style the region accordingly.', array('@region' => $region)),
      '#default_value' => theme_get_setting($region_key . '_style', $theme_key),
      '#options' => $region_style_options,
    );
  }

  // Navigational settings.
  $form['navigations'] = array(
    '#type' => 'fieldset',
    '#title' => t('Navigations'),
    '#description' => t('UIkit offers different types of navigations, like navigation bars and side navigations. Use breadcrumbs or a pagination to steer through articles.'),
    '#group' => 'uikit',
  );
  $form['navigations']['main_navbar'] = array(
    '#type' => 'fieldset',
    '#title' => t('Navigation bar'),
    '#description' => t('Configure settings for the navigation bar.'),
  );
  $form['navigations']['main_navbar']['navbar_container_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Navbar container'),
    '#description' => t('Configure settings for the navigation bar container.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['navigations']['main_navbar']['navbar_container_settings']['navbar_container'] = array(
    '#type' => 'checkbox',
    '#title' => t('Container'),
    '#description' => t('Add the .uk-container class to the navbar container to give it a max-width and wrap the navbar of your website. For large screens it applies a different max-width.'),
    '#default_value' => theme_get_setting('navbar_container', $theme_key),
  );
  $form['navigations']['main_navbar']['navbar_container_settings']['navbar_centering'] = array(
    '#type' => 'checkbox',
    '#title' => t('Centering'),
    '#description' => t('To center the navbar container, use the .uk-container-center class.'),
    '#default_value' => theme_get_setting('navbar_centering', $theme_key),
  );
  $form['navigations']['main_navbar']['navbar_container_settings']['navbar_attached'] = array(
    '#type' => 'checkbox',
    '#title' => t('Navbar attached'),
    '#description' => t("Adds the <code>.uk-navbar-attached</code> class to optimize the navbar's styling to be attached to the top of the viewport. For example, rounded corners will be removed."),
    '#default_value' => theme_get_setting('navbar_attached', $theme_key),
  );
  $form['navigations']['main_navbar']['navbar_margin'] = array(
    '#type' => 'fieldset',
    '#title' => t('Navbar margin'),
    '#description' => t('Configure the top and bottom margin to apply to the navbar.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['navigations']['main_navbar']['navbar_margin']['navbar_margin_top'] = array(
    '#type' => 'select',
    '#title' => t('Navbar top margin'),
    '#description' => t('Select the amount of top margin to apply to the navbar.'),
    '#default_value' => theme_get_setting('navbar_margin_top', $theme_key),
    '#options' => $navbar_margin_top_options,
  );
  $form['navigations']['main_navbar']['navbar_margin']['navbar_margin_bottom'] = array(
    '#type' => 'select',
    '#title' => t('Navbar bottom margin'),
    '#description' => t('Select the amount of bottom margin to apply to the navbar.'),
    '#default_value' => theme_get_setting('navbar_margin_bottom', $theme_key),
    '#options' => $navbar_margin_bottom_options,
  );
  $form['navigations']['main_navbar']['default_menus'] = array(
    '#type' => 'fieldset',
    '#title' => 'Default navbar menus',
    '#description' => t('Adjust settings for the default main and secondary menus in the navbar. Each system-generated and custom menu is available below, except menus assigned to the main and secondary source links.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['navigations']['main_navbar']['default_menus']['main_menu'] = array(
    '#type' => 'fieldset',
    '#title' => 'Main menu',
    '#description' => t('Adjust settings for the main menu in the navbar.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['navigations']['main_navbar']['default_menus']['main_menu']['main_menu_alignment'] = array(
    '#type' => 'select',
    '#title' => 'Main menu alignment',
    '#description' => t('Select whether to align the main menu to the left or right in the navbar.'),
    '#default_value' => theme_get_setting('main_menu_alignment', $theme_key),
    '#options' => array('Left', 'Right'),
  );
  $form['navigations']['main_navbar']['default_menus']['main_menu']['main_menu_dropdown_support'] = array(
    '#type' => 'checkbox',
    '#title' => 'Main menu dropdown support',
    '#description' => t('Select whether to add dropdown support to the main menu. NOTE: Dropdown functionality is only supported for 2 levels.<br /><span style="color: red;">NOTE:</span> Setting disabled. See <a href="https://www.drupal.org/node/2746097" target="_blank" style="background-color: #ddf; border-radius: 4px; padding: 2px 4px;">#2746097: Offcanvas menus display dropdown menus incorrectly</a>'),
    '#default_value' => theme_get_setting('main_menu_dropdown_support', $theme_key),
    '#attributes' => array(
      'disabled' => 'disabled',
    ),
  );
  $form['navigations']['main_navbar']['default_menus']['secondary_menu'] = array(
    '#type' => 'fieldset',
    '#title' => 'Secondary menu',
    '#description' => t('Adjust settings for the secondary menu in the navbar.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['navigations']['main_navbar']['default_menus']['secondary_menu']['secondary_menu_alignment'] = array(
    '#type' => 'select',
    '#title' => 'Secondary menu alignment',
    '#description' => t('Select whether to align the secondary menu to the left or right in the navbar.'),
    '#default_value' => theme_get_setting('secondary_menu_alignment', $theme_key),
    '#options' => array('Left', 'Right'),
  );
  $form['navigations']['main_navbar']['default_menus']['secondary_menu']['secondary_menu_dropdown_support'] = array(
    '#type' => 'checkbox',
    '#title' => 'Secondary menu dropdown support',
    '#description' => t('Select whether to add dropdown support to the secondary menu. NOTE: Dropdown functionality is only supported for 2 levels.<br /><span style="color: red;">NOTE:</span> Setting disabled. See <a href="https://www.drupal.org/node/2746097" target="_blank" style="background-color: #ddf; border-radius: 4px; padding: 2px 4px;">#2746097: Offcanvas menus display dropdown menus incorrectly</a>'),
    '#default_value' => theme_get_setting('secondary_menu_dropdown_support', $theme_key),
    '#attributes' => array(
      'disabled' => 'disabled',
    ),
  );
  $form['navigations']['main_navbar']['additional_navbar_menus'] = array(
    '#type' => 'fieldset',
    '#title' => 'Additional navbar menus',
    '#description' => t('Define and adjust settings for additional menus in the navbar.'),
  );

  foreach ($menus as $menu_name => $menu_title) {
    // Ignore the main and secondary menus, they will not be added dynamically.
    $ignore = array(
      $main_menu,
      $secondary_menu,
    );

    if (!in_array($menu_name, $ignore)) {
      // Gets the remaining menus not ignored and creates new settings for each.
      $menu_name = str_replace('-', '_', $menu_name);
      $menu_title = str_replace(' menu', '', $menu_title) . ' menu';

      $form['navigations']['main_navbar']['additional_navbar_menus'][$menu_name] = array(
        '#type' => 'fieldset',
        '#title' => t('@title', array('@title' => $menu_title)),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form['navigations']['main_navbar']['additional_navbar_menus'][$menu_name][$menu_name . '_in_navbar'] = array(
        '#type' => 'checkbox',
        '#title' => t('Include the @title in the navbar', array('@title' => $menu_title)),
        '#default_value' => theme_get_setting($menu_name . '_in_navbar', $theme_key),
      );
      $form['navigations']['main_navbar']['additional_navbar_menus'][$menu_name][$menu_name . '_additional_alignment'] = array(
        '#type' => 'select',
        '#description' => t('Select whether to align the @menu to the left or right in the navbar.', array('@menu' => $menu_title)),
        '#default_value' => theme_get_setting($menu_name . '_alignment', $theme_key),
        '#options' => array('Left', 'Right'),
      );
    }
  }
  $form['navigations']['local_tasks'] = array(
    '#type' => 'fieldset',
    '#title' => t('Local tasks'),
    '#description' => t('Configure settings for the local tasks menus.'),
  );
  $form['navigations']['local_tasks']['primary_tasks'] = array(
    '#type' => 'container',
  );
  $form['navigations']['local_tasks']['primary_tasks']['primary_tasks_demo'] = array(
    '#markup' => '<div id="primary-tasks-demo" class="uk-admin-demo">' . $demo_local_tasks . '</div>',
  );
  $form['navigations']['local_tasks']['primary_tasks']['primary_tasks_style'] = array(
    '#type' => 'select',
    '#title' => t('Primary tasks style'),
    '#description' => t('Select the style to apply to the primary local tasks.'),
    '#default_value' => theme_get_setting('primary_tasks_style', $theme_key),
    '#options' => $subnav_options,
  );
  $form['navigations']['local_tasks']['secondary_tasks'] = array(
    '#type' => 'container',
  );
  $form['navigations']['local_tasks']['secondary_tasks']['secondary_tasks_demo'] = array(
    '#markup' => '<div id="secondary-tasks-demo" class="uk-admin-demo">' . $demo_local_tasks . '</div>',
  );
  $form['navigations']['local_tasks']['secondary_tasks']['secondary_tasks_style'] = array(
    '#type' => 'select',
    '#title' => t('Secondary tasks style'),
    '#description' => t('Select the style to apply to the secondary local tasks.'),
    '#default_value' => theme_get_setting('secondary_tasks_style', $theme_key),
    '#options' => $subnav_options,
  );
  $form['navigations']['breadcrumb'] = array(
    '#type' => 'fieldset',
    '#title' => t('Breadcrumbs'),
    '#description' => t('Configure settings for breadcrumb navigation.'),
  );
  $form['navigations']['breadcrumb']['display_breadcrumbs'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display breadcrumbs'),
    '#description' => t('Check this box to display the breadcrumb.'),
    '#default_value' => theme_get_setting('display_breadcrumbs', $theme_key),
  );
  $form['navigations']['breadcrumb']['breakcrumbs_home_link'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display home link in breadcrumbs'),
    '#description' => t('Check this box to display the home link in breadcrumb trail.'),
    '#default_value' => theme_get_setting('breakcrumbs_home_link', $theme_key),
  );

  // Basic elements.
  $form['elements'] = array(
    '#type' => 'fieldset',
    '#title' => t('Elements'),
    '#description' => t("Style basic HTML elements, like tables and forms. These components use their own classes. They won't interfere with any default element styling."),
    '#group' => 'uikit',
  );

  // Common components.
  $form['common'] = array(
    '#type' => 'fieldset',
    '#title' => t('Common'),
    '#description' => t("Here you'll find components that you often use within your content, like buttons, icons, badges, overlays, animations and much more."),
    '#group' => 'uikit',
  );

  // Javascript components.
  $form['javascript'] = array(
    '#type' => 'fieldset',
    '#title' => t('Javascript'),
    '#description' => t('These components rely mostly on JavaScript to fade in hidden content, like dropdowns, modal dialogs, off-canvas bars and tooltips.'),
    '#group' => 'uikit',
  );

  // Advanced components.
  $form['components'] = array(
    '#type' => 'fieldset',
    '#title' => t('Components'),
    '#description' => t("UIkit offers some advanced components that are not included in the UIkit core framework. Usually you wouldn't use these components in your everyday website."),
    '#group' => 'uikit',
  );

  // Create vertical tabs to place Drupal's default theme settings in.
  $form['basic_settings'] = array(
    '#type' => 'vertical_tabs',
    '#prefix' => '<h3>' . t('Basic Settings') . '</h3>',
    '#weight' => 0,
  );

  // Group Drupal's default theme settings in the basic settings.
  $form['theme_settings']['#group'] = 'basic_settings';
  $form['logo']['#group'] = 'basic_settings';
  $form['logo']['#attributes']['class'] = array();
  $form['favicon']['#group'] = 'basic_settings';
}

/**
 * Callback function to validate the viewport width.
 *
 * The default value for the initial scale is turned off, but it is recommended
 * to use the literal device width. If the user chooses to define a pixel width,
 * the value for the initial scale needs to be validated to ensure only an
 * integer is entered.
 */
function _uikit_viewport_custom_width_validate($element, &$form_state) {
  $device_width_ratio = $form_state['values']['viewport_device_width_ratio'] == 1;

  if ($device_width_ratio && empty($element['#value'])) {
    form_set_error($element['#name'], t('<b>Other</b> was selected for <b>Device width ratio</b>, but no value was given for <b>Custom device width</b>. Please enter an integer value in <b>Custom device width</b> under <b>Mobile settings</b> and save the configuration.'));
  }
  elseif ($device_width_ratio && !empty($element['#value']) && !ctype_digit($element['#value'])) {
    form_set_error($element['#name'], t('<b>Custom device width</b> can only contain an integer number, without a decimal point. Please check the value for <b>Custom device width</b> under <b>Mobile settings</b> and save the configuration.'));
  }
}

/**
 * Callback function to validate the viewport height.
 *
 * The default value for the initial scale is turned off, but it is recommended
 * to use the literal device width. If the user chooses to define a pixel width,
 * the value for the initial scale needs to be validated to ensure only an
 * integer is entered.
 */
function _uikit_viewport_custom_height_validate($element, &$form_state) {
  $device_height_ratio = $form_state['values']['viewport_device_height_ratio'] == 1;

  if ($device_height_ratio && empty($element['#value'])) {
    form_set_error($element['#name'], t('<b>Other</b> was selected for <b>Device height ratio</b>, but no value was given for <b>Custom device height</b>. Please enter an integer value in <b>Custom device height</b> under <b>Mobile settings</b> and save the configuration.'));
  }
  elseif ($device_height_ratio && !empty($element['#value']) && !ctype_digit($element['#value'])) {
    form_set_error($element['#name'], t('<b>Custom device height</b> can only contain an integer number, without a decimal point. Please check the value for <b>Custom device height</b> under <b>Mobile settings</b> and save the configuration.'));
  }
}
