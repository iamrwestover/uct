<?php
$view_mode = !empty($form['#av_view_mode']);
?>
<div class="uk-grid uk-grid-collapse" style="position: relative;">
  <?php print drupal_render($form['badge']); ?>
  <div class="uk-width-5-10">

    <div class="uk-grid uk-grid-collapse">
      <div class="uk-width-1-2 uk-text-center" style="width: 7%;">
        <div class="av-nestable-cell">
          <span class="av-nestable-row-num" style="<?php print $view_mode ? 'top: 0;' : ''; ?>"><?php print $form['#prod_index'] + 1; ?></span>
        </div>
      </div>
      <div class="uk-width-1-2" style="width: 93%;">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['product_title']); ?>
        </div>
      </div>
    </div>
  </div>

  <div class="uk-width-5-10">

    <div class="uk-grid uk-grid-collapse">

      <div class="uk-width-1-5">
        <div class="av-nestable-cell uk-text-center">
          <?php print drupal_render($form['onhand_qty']); ?>
        </div>
      </div>
      <div class="uk-width-1-5">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['new_qty']); ?>
        </div>
      </div>

      <div class="uk-width-1-5">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['qty_diff']); ?>
        </div>
      </div>
      <div class="uk-width-<?php print ($view_mode ? '2' : '1'); ?>-5">
        <div class="av-nestable-cell uk-text-center">
          <?php print drupal_render($form['uom_title_fixed']); ?>
        </div>
      </div>
      <?php if (empty($view_mode)): ?>
        <div class="uk-width-1-5 uk-text-center">
          <div class="av-nestable-cell">
            <?php print drupal_render($form['prod_delete_btn']); ?>
          </div>
        </div>
      <?php endif; ?>
    </div>

  </div>
</div>
