<h2 id="new-database"><?php print $help_title; ?></h2>
<div class="uk-alert uk-alert-warning">
  <div class="uk-text-bold">This operation will overwrite the entire system database.</div>
  Make sure that you know what you are doing and that you have backups ready.
  <br />Be careful and proceed at your own risk.
</div>
<p>Before proceeding, it is paramount that you <?php print l('create a backup', 'av/help', array('fragment' => 'backup-create')); ?> of EVERYTHING IN THE DATABASE first.</p>
<p>To reset the system with a new database, you have two options:</p>
<ul>
  <li>Wipe transactions only. Customers, Vendors, Branches, and Products data will be kept. Chart of Accounts and System Users will remain intact too.</li>
  <li>Wipe everything. Only Chart of Accounts and System Users will remain intact.</li>
</ul>

<h3>Wipe transactions only</h3>
<p>To wipe all transactions, carefully do the following:</p>
<ol>
  <li>Make sure no one else is using the system.</li>
  <li><?php print l('Create a backup', 'av/help', array('fragment' => 'backup-create')); ?>. Make sure you select "Everything in the database EXCEPT Transactions" option.</li>
  <li><?php print l('Restore the backup', 'av/help', array('fragment' => 'backup-restore')); ?>.</li>
</ol>
<p>After this, the system will be in a state where <span class="uk-text-warning">ALL TRANSACTIONS ARE WIPED</span> - effectively having a new database - but still having existing users, chart of accounts, customers, branches, vendors, and product data intact.</p>

<h3>Wipe everything</h3>
<p>To wipe everything, carefully do the following:</p>
<ol>
  <li>Make sure no one else is using the system.</li>
  <li><?php print l('Create a backup', 'av/help', array('fragment' => 'backup-create')); ?>. Select "Clean Install - only Chart of Accounts and system users will be backed up" option.</li>
  <li><?php print l('Restore the backup', 'av/help', array('fragment' => 'backup-restore')); ?>.</li>
</ol>
<p>After this, the system will be in a state where <span class="uk-text-warning">EVERYTHING IS WIPED</span> - transactions, customers, branches, vendors, and product data <span class="uk-text-bold uk-text-warning">will be gone</span> - effectively having a new database - but still having existing users and chart of accounts intact. </p>
