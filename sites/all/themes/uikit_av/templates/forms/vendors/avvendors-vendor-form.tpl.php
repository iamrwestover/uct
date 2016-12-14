<?php
$view_mode = !empty($form['#av_view_mode']) ;
$vertical_margin_class = $view_mode ? 'uk-margin-bottom' : 'uk-margin-small-bottom';

// Remove fieldset titles.
$form['info']['#access'] = FALSE;
$form['contact']['#access'] = FALSE;
$form['address']['#access'] = FALSE;
$form['payment']['#access'] = FALSE;
$form['misc']['#access'] = FALSE;

// Remove resize handle from textareas.
$form['address']['address']['#resizable'] = FALSE;
$form['address']['address']['#attributes']['rows'] = 3;

// Misc.
$form['address']['address']['#attributes']['placeholder'] = 'Building / Unit # / Street / etc.';
?>

<div class="uk-grid uk-grid-large">
  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-2-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['company']); ?></div>
      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['category_id']); ?></div>

      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['first_name']); ?></div>
      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['middle_name']); ?></div>
      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['last_name']); ?></div>

      <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['display_name']); ?></div>
      <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['agent_id']); ?></div>
    </div>
  </div>


  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['contact']['email']); ?></div>
      <div class="uk-width-1-2 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['contact']['contact_number']); ?></div>

      <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['contact']['website']); ?></div>

      <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['toggle_me']); ?></div>
      <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['settings']); ?></div>
      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['term_id']); ?></div>
      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['discount_type']); ?></div>
      <div class="uk-width-1-3 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['info']['discount_value']); ?></div>
    </div>
  </div>

  <div class="uk-width-1-1 uk-margin-top">
    <ul class="uk-tab" data-uk-tab="{connect:'#more-info'}">
      <li><a href="">Address</a></li>
      <li><a href="">Miscellaneous</a></li>
    </ul>
    <ul id="more-info" class="uk-switcher">
      <li class="uk-panel uk-panel-box">
        <div class="uk-grid">
          <div class="uk-width-1-1 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['address']); ?></div>
          <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['city']); ?></div>
          <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['province']); ?></div>
          <div class="uk-width-2-6 <?php print $vertical_margin_class; ?>"><?php print drupal_render($form['address']['zip_code']); ?></div>
        </div>
      </li>
      <li class="uk-panel uk-panel-box"><?php print drupal_render($form['misc']['notes']); ?></li>
    </ul>
  </div>
</div>



<?php
$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/vendors', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);