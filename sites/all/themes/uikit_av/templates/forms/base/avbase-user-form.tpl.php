<?php
$form['picture']['#title'] = '';
?>
<div class="uk-grid">
  <div class="uk-width-1-3"><?php print drupal_render($form['account']['name']); ?></div>
  <div class="uk-width-1-3"><?php print drupal_render($form['account']['mail']); ?></div>
  <div class="uk-width-1-3"></div>

  <div class="uk-width-1-3 uk-margin-top"><?php print drupal_render($form['avuser']['first_name']); ?></div>
  <div class="uk-width-1-3 uk-margin-top"><?php print drupal_render($form['avuser']['last_name']); ?></div>
  <div class="uk-width-1-3 uk-margin-top"><?php print drupal_render($form['avuser']['contact_number']); ?></div>
  <!--<div class="uk-width-1-3"></div>-->
  <!--<div class="uk-width-1-3"></div>-->

  <div class="uk-width-1-3 uk-margin-top"><?php print drupal_render($form['account']['pass']); ?></div>
  <div class="uk-width-2-3 uk-margin-top"><?php print drupal_render($form['avuser']['address']); ?></div>

  <div class="uk-width-1-3">
      <div class="uk-grid">
        <div class="uk-width-1-1 uk-margin-top"><?php print drupal_render($form['account']['status']); ?></div>
        <div class="uk-width-1-1 uk-margin-top"><?php print drupal_render($form['account']['roles']); ?></div>
      </div>
  </div>
  <div class="uk-width-2-3">
    <div class="uk-width-1-2 uk-margin-top"><?php print drupal_render($form['picture']); ?></div>
  </div>


</div>
<?php
$form['actions']['go_back']['#markup'] = l('Cancel', 'av/users', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);