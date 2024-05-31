# Egaroucid ダウンロード

インストール版とZip版があります。



## ダウンロード

Egaroucidはx64のCPU(Intel製かAMD製)に対応しています。ARMのCPUを使っている場合は動きません。

EgaroucidはSIMDバージョン(AVX2が必要)に最適化して作っていますが、こちらは概ね2013年以降のCPUでないと動作しません。その場合にはGenericバージョンを使用してください。また、AVX-512に対応したCPU (Intel Core iシリーズの第7から11世代など) をお使いの場合はAVX512版の方が高速に動くかもしれません。

以下から自分の環境に合ったものをダウンロードしてください。



DOWNLOAD_TABLE_HERE



過去のバージョンや各バージョンのリリースノートは[GitHubのリリース](https://github.com/Nyanyan/Egaroucid/releases)からご覧ください。



## インストール

インストール版の場合、ダウンロードしたインストーラを実行してください。管理者権限が必要です。



「WindowsによってPCが保護されました」と出た場合は、「詳細情報」をクリックすると実行することができます。ただし、この操作は自己責任で行ってください。

<div class="centering_box">
    <img class="pic2" src="img/cant_run1.png" alt="「WindowsによってPCが保護されました」という画面">
    <img class="pic2" src="img/cant_run2.png" alt="「WindowsによってPCが保護されました」という画面において「詳細情報」を押して実行する">
</div>




## 実行

インストールまたは解凍した<code>Egaroucid_[バージョン情報].exe</code>を実行するとEgaroucidが起動します。

<div class="centering_box">
    <img class="pic2" src="img/egaroucid.png" alt="Egaroucid">
</div>


## book形式の変更について

### egbk3形式への変更

Egaroucidはバージョン6.5.1からbook形式を変更し、拡張子が```.egbk3```のものを使うようになりました。以前のバージョンをお使いで新しく6.5.1をインストールした場合、初回起動時に古い```.egbk2```形式および```.egbk```形式のbookを自動で変換します。また、古い形式もbookの読み込みなど各種機能が使えます。ただし、保存形式は新しい```.egbk3```形式になります。

### egbk2形式への変更

Egaroucidはバージョン6.3.0からbook形式を変更しました。新しいbookの拡張子は```.egbk2```です。6.2.0以前のバージョンをお使いで新しく6.3.0をインストールした場合、初回起動時に古い```.egbk```形式のbookを自動で変換します。また、古い形式もbookの参照および統合機能が使えます。ただし、保存形式は新しい```.egbk2```形式になります。
