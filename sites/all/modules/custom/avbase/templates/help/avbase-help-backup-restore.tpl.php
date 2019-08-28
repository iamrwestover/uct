<h2 id="backup-restore"><?php print $help_title; ?></h2>
<div class="uk-alert uk-alert-warning">
  <div class="uk-text-bold">This operation will overwrite the entire system database.</div>
  Make sure that you know what you are doing and that you have backups ready.
  <br />Be careful and proceed at your own risk.
</div>
<p>It is <span class="uk-text-warning">CRITICAL</span> that no one else is using the system during restore.</p>
<ol>
  <li>Make sure you <?php print l('create a backup', 'av/help', array('fragment' => 'backup-create')); ?> of EVERYTHING IN THE DATABASE first before proceeding.</li>
  <li>Go to <?php print l('Restore page', 'admin/config/system/backup_migrate/restore', array('attributes' => array('target' => '_blank'))); ?>.</li>
  <li>Find and select your <code>.mysql.gz</code> backup file</li>
  <li>Under <span class="uk-text-bold">Restore to</span>, select "Default database".</li>
  <li>Click the <span class="uk-text-bold">Restore now</span> button</li>
  <li>The restore will take a while to finish. After that, <?php print l('Flush the system cache', 'update.php', array('attributes' => array('target' => '_blank'))); ?>.</li>
</ol>
