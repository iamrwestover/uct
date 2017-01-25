
<div class="uk-grid uk-grid-small uk-margin-top">
  <div class="uk-width-3-10">
    <?php print drupal_render($form['search_text']); ?>
  </div>
  <div class="uk-width-1-10">
    <?php print drupal_render($form['avprod.category_id']); ?>
  </div>
  <div class="uk-width-5-10">
    <?php print drupal_render($form['low_stock']); ?>
    <?php print drupal_render($form['no_stock']); ?>
    <?php print drupal_render($form['buttons']['reset_button']); ?>
    <?php print drupal_render($form['buttons']['search_button']); ?>
    <?php print drupal_render($form['settings']['page_num']); ?>
  </div>
</div>
<?php
print drupal_render_children($form);
