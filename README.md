# CW Decoder for Raspberry Pi Pico 2

Raspberry Pi Pico 2（RP2350）と2.8インチ ST7789 LCDで動作するCW Decoderです。
VS Code、PlatformIO、Arduino-Pico、LovyanGFXでビルドします。
Adafruit系ライブラリとタッチスクリーンは使用しません。

現在のバージョンは `v2.1` です。

CH32V003 / CH32V006（UIAPduino）版は
[UIAP_CWDecoder2](https://github.com/pokibon3/UIAP_CWDecoder2) を参照してください。
モールス復号とノイズ対策は同プロジェクトのv2.0ロジックを引き継いでいます。

## 対応ハードウェア

- Raspberry Pi Pico 2（RP2350、150MHzデュアルコア）
- LCD: ST7789 240x320、SPI接続、320x240横表示
- オーディオ入力: GP26 / ADC0、8kHzサンプリング
- 操作: 最大5個のプッシュスイッチ（GNDへ接続、内部プルアップ使用）

## 機能

モールス信号（欧文／和文）をオーディオ入力からデコードし、LCDに表示します。

- ステータス: US/JP、WPM、選択トーン
- デコード文字: 16列×6行、欧文全角表示、和文カタカナ対応
- FFTスペクトラム: 約300〜1200Hz、ピーク周波数、選択トーン帯域
- オシロスコープ: 生波形、トーンエンベロープ、キー判定
- TONE AUTO: 550〜1000Hzの信号へ自動同調
- 手動トーン: 600 / 700 / 800 / 900 / 1000Hz

## 信号処理

- GP26 / ADC0を8kHzで連続サンプリング
- 48サンプル×2面の連結DMAにより6ms単位で処理
- 3ビンfloat Goertzelによるトーン検出
- 96サンプル窓（通常）／48サンプル窓（高速CW）をWPMに応じて切り替え
- 中心／サイド比、ヒステリシス、ノイズブランカによる誤検出抑制
- ギャップを含む短点長推定とWPM追従
- 256点FFTはスペクトラム表示とAUTO同調に使用
- Core 0: LCD描画とスイッチ処理
- Core 1: ADC、Goertzel、FFT、モールス復号

## プッシュスイッチ操作

すべてのスイッチはGPIOとGNDの間に接続します。

| スイッチ | GPIO | 操作 |
|---|---:|---|
| SW1 | GP14 | US / JP切り替え |
| SW2 | GP15 | トーンを次へ |
| SW3 | GP13 | KEY表示 ON / OFF |
| SW4 | GP10 | ENV表示 ON / OFF |
| SW5 | GP11 | RAW表示 ON / OFF |

トーンの選択順は `AUTO → 600 → 700 → 800 → 900 → 1000Hz` です。
SW2を押すたびに順方向へ移動します。これにより、ESP32版でFFTパネルを
タップしていたトーン選択もプッシュスイッチだけで操作できます。

## 配線

### ST7789 LCD

| Pico 2物理ピン | GPIO | LCD | 用途 |
|---:|---|---|---|
| 38 | GND | GND | GND |
| 36 | 3V3(OUT) | VCC | 電源 |
| 24 | GP18 | SCL | SPI0 SCK |
| 25 | GP19 | SDA | SPI0 MOSI |
| 26 | GP20 | RST | Reset |
| 21 | GP16 | DC | Data / Command |
| 22 | GP17 | CS | Chip Select |
| 36 | 3V3(OUT) | BL | Backlight |

### その他

| Pico 2物理ピン | GPIO | 用途 |
|---:|---|---|
| 31 | GP26 / ADC0 | MIC / AF入力 |
| 19 | GP14 | SW1 |
| 20 | GP15 | SW2 |
| 17 | GP13 | SW3 |
| 14 | GP10 | SW4（追加） |
| 15 | GP11 | SW5（追加） |
| 内蔵 | GP25 | CW状態LED |
| 16 | GP12 | DSP処理タイミングTEST出力 |

配線の詳細とAF入力保護・バイアス回路は
[`hardware/wiring/CW_配線図.pptx`](hardware/wiring/CW_%E9%85%8D%E7%B7%9A%E5%9B%B3.pptx)を参照してください。
ADC入力電圧は0〜3.3Vの範囲を超えないようにしてください。

## ハードウェア資料

- `hardware/wiring/`: 配線図とLCD寸法資料
- `hardware/case/`: ケースの3MFデータとLCD機械寸法DXF

元写真・動画と作業用の配線図コピーはリポジトリに含めません。

## ビルド

1. VS Codeでこのフォルダを開く
2. PlatformIO IDEを有効にする
3. PlatformIOツールバーから `Build` を実行する

environmentは `rpipico2` です。コマンドラインでは次を実行できます。

```sh
pio run
```

生成される書き込み用ファイル:

```text
.pio/build/rpipico2/firmware.uf2
```

初回はBOOTSELを押しながらPico 2をUSB接続し、表示されたドライブへUF2をコピーします。

## ソフトウェア構成

- `src/audio.cpp`: RP2350 ADCと連結DMA
- `src/dsp.cpp`: Goertzel、FFT、AUTO同調、スコープデータ
- `src/decoder.cpp`: CW状態機械、ノイズ除去、WPM推定
- `src/decode.cpp`: 欧文／和文モールス符号表
- `src/display.cpp`: ST7789表示と文字バッファ
- `src/main.cpp`: デュアルコア起動と5スイッチ操作
- `src/board_config.h`: Pico 2のピン定義

## ライセンス

モールスデコーダ部分はHjalmar Skovholm Hansen OZ1JHM氏のArduino用CW Decoderを
一部流用しており、GNU General Public License（GPL）が適用されます。
元ソースと上流リポジトリでGPLのバージョンが明記されていないため、
公開リポジトリの正式な`LICENSE`は確認後に追加します。

利用するライブラリと開発環境のライセンスは
[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)を参照してください。

- Original: [http://oz1jhm.dk/content/very-simpel-cw-decoder-easy-build](http://oz1jhm.dk/content/very-simpel-cw-decoder-easy-build)
- GPL: [http://www.gnu.org/copyleft/gpl.html](http://www.gnu.org/copyleft/gpl.html)
