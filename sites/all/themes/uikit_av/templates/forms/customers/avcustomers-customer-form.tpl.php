<?php
// Remove fieldset titles.
unset($form['info']['#title']);
unset($form['contact']['#title']);
unset($form['address']['#title']);
unset($form['payment']['#title']);
unset($form['misc']['#title']);

//unset($form['contact']['email']['#title']);
//unset($form['contact']['contact_number']['#title']);
//unset($form['contact']['website']['#title']);
//$form['contact']['email']['#attributes']['placeholder'] = 'e-mail address';
//$form['contact']['contact_number']['#attributes']['placeholder'] = 'contact number';
//$form['contact']['website']['#attributes']['placeholder'] = 'website';

// Wrap buttons.
$form['buttons']['#prefix'] = '<div class="uk-margin-top">';
$form['buttons']['#suffix'] = '</div>';

// Remove resize handle from textareas.
//$form['misc']['notes']['#resizable'] = FALSE;
$form['address']['address']['#resizable'] = FALSE;
$form['address']['address']['#attributes']['rows'] = 3;

// Simplify address fields.
//unset($form['address']['city']['#title']);
//unset($form['address']['province']['#title']);
//unset($form['address']['zip_code']['#title']);
$form['address']['address']['#attributes']['placeholder'] = 'Building / Unit # / Street / etc.';
//$form['address']['city']['#attributes']['placeholder'] = 'City';
//$form['address']['province']['#attributes']['placeholder'] = 'Province';
//$form['address']['zip_code']['#attributes']['placeholder'] = 'ZIP Code';
?>

<div class="uk-grid uk-grid-large">
  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-1 uk-margin-small-bottom"><?php print drupal_render($form['info']['company']); ?></div>

      <div class="uk-width-1-10 uk-margin-small-bottom"><?php print drupal_render($form['info']['title']); ?></div>
      <div class="uk-width-3-10 uk-margin-small-bottom"><?php print drupal_render($form['info']['first_name']); ?></div>
      <div class="uk-width-3-10 uk-margin-small-bottom"><?php print drupal_render($form['info']['middle_name']); ?></div>
      <div class="uk-width-3-10 uk-margin-small-bottom"><?php print drupal_render($form['info']['last_name']); ?></div>

      <div class="uk-width-1-1 uk-margin-small-bottom"><?php print drupal_render($form['info']['display_name']); ?></div>
    </div>
  </div>

  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-2 uk-margin-small-bottom"><?php print drupal_render($form['contact']['email']); ?></div>
      <div class="uk-width-1-2 uk-margin-small-bottom"><?php print drupal_render($form['contact']['contact_number']); ?></div>

      <div class="uk-width-1-1 uk-margin-small-bottom"><?php print drupal_render($form['contact']['website']); ?></div>
    </div>
  </div>

  <div class="uk-width-1-1 uk-margin-top">
    <ul class="uk-tab" data-uk-tab="{connect:'#more-info'}">
      <li><a href="">Address</a></li>
      <li><a href="">Payment</a></li>
      <li><a href="">Notes</a></li>
    </ul>
    <ul id="more-info" class="uk-switcher">
      <li class="uk-panel uk-panel-box">

        <div class="uk-grid">
          <div class="uk-width-1-2">
            <div class="uk-grid uk-grid-small">
              <div class="uk-width-1-1 uk-margin-small-bottom"><?php print drupal_render($form['address']['address']); ?></div>
              <div class="uk-width-2-6 uk-margin-small-bottom"><?php print drupal_render($form['address']['city']); ?></div>
              <div class="uk-width-2-6 uk-margin-small-bottom"><?php print drupal_render($form['address']['province']); ?></div>
              <div class="uk-width-2-6 uk-margin-small-bottom"><?php print drupal_render($form['address']['zip_code']); ?></div>
            </div>
          </div>

          <div class="uk-width-1-2">
            <?php print drupal_render($form['address']['same_addr']); ?>
          </div>
        </div>

      </li>
      <li class="uk-panel uk-panel-box"><?php print drupal_render($form['payment']); ?></li>
      <li class="uk-panel uk-panel-box"><?php print drupal_render($form['misc']['notes']); ?></li>
    </ul>
  </div>
</div>

<?php
$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/customers', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);