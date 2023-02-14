# モータードライバー

## 外観
こんなやつ  

<img src=https://github.com/laika90/shinjinkennsyu_4/blob/master/document/images/motor_diver_01.jpeg width="320px">  

（[秋月電子通商](https://akizukidenshi.com/catalog/g/gK-11219/)より）  

これはキットになっていますが、この黒いチップみたいなのが本体です。周りのやつはモータードライバとは呼びません。たぶん。

## なんこれ

「モータードライバ」と言えば、モーターを制御するパーツのことです。  

しかし偏にモーターと言っても、ブラシレスモーター、ブラシモーター、ステッピングモーターなど色々あり、それぞれモーターでの動力の得方が異なります。（つまり、名前が違うだけに、中身ももちろん違います。）  

ですから、それぞれのモーターに対応するモータードライバというものをちゃんと買わなきゃいけません。  

今回買ったのはTB6612使用のモータードライバです（名前はどうでもいいです。調べるときに使うくらい。名前が違うなら、品種も違うから使い方も違う、程度に思ってください。）。

## なんでこれが必要なの？

さて、モーター制御の理由ですが、例えばモータードライバを使わないで制御する場合のことを考えてみましょう。（今回使うのはDCモーターですので、DCモーターに限って話します。）  

モーターは基本電池を繋げれば動きますから、電池を直で繋いでみましょう。  

するとモーターは動き出します、、、、が、パワー全開です。出力の調整は？抵抗を噛ませたりするのでしょうか。  

さらに、逆回転したかったらどうなんでしょう。DCモーターからは２本コードが出ているのはなんとなく知っていると思います。

<img src=https://github.com/laika90/shinjinkennsyu_4/blob/master/document/images/motor_diver_02.jpeg width="320px">  

（[秋月電子通商](https://akizukidenshi.com/catalog/g/gP-09169/)より）  

DCモーターはこの2本を今とは逆に接続すれば、逆回転できます。これは手作業です。ロボット作るときにはハードではなくソフトで設計したいですよね。  

さらにさらに、モーター（出なくとも電子部品）には定格電圧というものが存在しています。これは、これ以上電圧かけないでね！という値です。定格電圧以上の電圧をかけるとぶっ壊れます。

今は電池に直挿ししているので、まぁ定格電圧を超えない電池を選べば良いでしょう。ですが、おいおいマイコンを使いたいとき、給電も同時にしたいですよね。となると電池2積み？となってしまいます。

また、DCモーターはその性質上、モーター部分を回しても電流が流れます。なのでマイコンや他の素子とモーターがつながっているときに、なんらかの原因でモーターが回ってしまうと、逆に電流が流れてマイコンや素子の破損の危険性があります。  

ということで問題点は以下のような感じです。  

* 出力の微調整が無理
* 逆回転が手作業ゲー
* 電池が余計
* ちょっと危険

さて、これらの問題を全て解決してくれるのがモータードライバです。天才かな？

## 仕組みと使い方



