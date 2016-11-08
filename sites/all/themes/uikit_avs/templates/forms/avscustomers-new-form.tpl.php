<?php
// Remove fieldset titles.
unset($form['info']['#title']);
unset($form['contact']['#title']);
unset($form['address']['#title']);
unset($form['payment']['#title']);

//unset($form['contact']['email']['#title']);
//unset($form['contact']['contact_number']['#title']);
//unset($form['contact']['website']['#title']);
//$form['contact']['email']['#attributes']['placeholder'] = 'e-mail address';
//$form['contact']['contact_number']['#attributes']['placeholder'] = 'contact number';
//$form['contact']['website']['#attributes']['placeholder'] = 'website';

// Wrap buttons.
$form['buttons']['#prefix'] = '<div class="uk-margin-top">';
$form['buttons']['#suffix'] = '</div>';

$form['address']['address']['#resizable'] = FALSE;
$form['address']['address']['#attributes']['rows'] = 3;
?>

<div class="uk-grid uk-grid-large uk-grid-divider">
  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-10"><?php print drupal_render($form['info']['title']); ?></div>
      <div class="uk-width-3-10"><?php print drupal_render($form['info']['first_name']); ?></div>
      <div class="uk-width-3-10"><?php print drupal_render($form['info']['middle_name']); ?></div>
      <div class="uk-width-3-10"><?php print drupal_render($form['info']['last_name']); ?></div>
    </div>
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-2"><?php print drupal_render($form['info']['company']); ?></div>
      <div class="uk-width-1-2"><?php print drupal_render($form['info']['display_name']); ?></div>
    </div>
  </div>

  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-2">
        <?php print drupal_render($form['contact']['email']); ?>
        <?php print drupal_render($form['contact']['website']); ?>
      </div>
      <div class="uk-width-1-2">
        <?php print drupal_render($form['contact']['contact_number']); ?>
      </div>
    </div>
  </div>
</div>

<div class="uk-margin-top">
  <ul class="uk-tab" data-uk-tab="{connect:'#more-info'}">
    <li><a href="">Address</a></li>
    <li><a href="">Payment</a></li>
    <li><a href="">Notes</a></li>
  </ul>
  <ul id="more-info" class="uk-switcher">
    <li class="uk-panel uk-panel-box">
      <div class="uk-grid uk-grid-width-1-2 uk-grid-large">
        <div>
          <div class="uk-grid-small"><?php print drupal_render($form['address']['address']); ?></div>
          <div class="uk-grid uk-grid-width-1-3 uk-grid-small">
            <div><?php print drupal_render($form['address']['city']); ?></div>
            <div><?php print drupal_render($form['address']['province']); ?></div>
            <div><?php print drupal_render($form['address']['zip_code']); ?></div>
          </div>
        </div>
        <div>
          <?php print render($form['address']['same_addr']); ?>
        </div>
      </div>
    </li>
    <li class="uk-panel uk-panel-box"><div><?php print drupal_render($form['payment']); ?></div></li>
    <li class="uk-panel uk-panel-box">Notes!</li>
  </ul>
</div>

<?php
print drupal_render_children($form);
?>