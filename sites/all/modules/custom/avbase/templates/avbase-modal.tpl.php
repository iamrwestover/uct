<?php
  $close_btn_label = isset($close_btn_label) ? $close_btn_label : t('Cancel');
  $onclick_script = isset($onclick_script) ? $onclick_script : '';
  $modal_classes = isset($modal_classes) ? (' ' . implode(' ', $modal_classes)) : '';
?>
<a id="uk-modal-toggle-<?php print $modal_id; ?>" class="uk-hidden">Modal toggle</a>
<div id="uk-modal-<?php print $modal_id; ?>" class="uk-modal">
  <div class="uk-modal-dialog<?php print $modal_classes; ?>">
    <?php if (!empty($show_close_button)): ?>
      <a class="uk-modal-close uk-close"></a>
    <?php endif; ?>
    <?php if (!empty($modal_title)): ?>
      <div class="uk-modal-header"><h2><?php print $modal_title; ?></h2></div>
    <?php endif; ?>
    <div id="uk-modal-<?php print $modal_id; ?>-body">Come and join the black parade.</div>
    <div class="uk-modal-footer uk-text-right">
      <button type="button" class="uk-button uk-button-primary uk-button-modal-ok">Continue</button>
      <button type="button" class="uk-button uk-modal-close"><?php print $close_btn_label; ?></button>
    </div>
  </div>
</div>