
<div class="uk-grid uk-grid-small">
  <div class="uk-width-3-10">
    <div class="uk-panel uk-panel-box uk-height-1-1" style="background: #fff;">
      <?php print drupal_render($form['search']); ?>
    </div>
  </div>


  <div class="uk-width-medium-7-10">
    <div class="uk-panel uk-panel-box uk-height-1-1" style="background: #fff;">

      <!--<div class="uk-text-right" style="position: absolute; top: -52px; right: 0;">-->
      <!--  --><?php //print drupal_render($form['buttons']); ?>
      <!--</div>-->

      <div>
        <?php print drupal_render($form['entity_info']); ?>
      </div>
    </div>
  </div>
</div>

<?php print drupal_render_children($form); ?>