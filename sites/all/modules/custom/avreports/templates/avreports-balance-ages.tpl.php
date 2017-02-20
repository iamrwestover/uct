<!--<hr />-->
<!--<hr class="uk-margin-small-bottom" />-->
<div class="uk-panel uk-panel-box">
  <div class="uk-margin-bottom uk-text-center">
    <span class="uk-margin-small-right">Outstanding balance:</span><em><?php print $form['#total_balance']; ?></em>
  </div>
  <div class="uk-grid uk-grid-small uk-text-center" data-uk-grid-margin>
    <?php foreach (element_children($form) as $balance_key): ?>
      <div class="uk-width-1-6">
        <?php print drupal_render($form[$balance_key]); ?>
      </div>
    <?php endforeach; ?>
  </div>
</div>
<!--<hr />-->