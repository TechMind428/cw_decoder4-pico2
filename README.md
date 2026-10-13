# CW Decoder for Raspberry Pi Pico 2

Raspberry Pi Pico 2（RP2350）と2.8インチ ST7789 LCDで動作するCW Decoderです。
VS Code、PlatformIO、Arduino-Pico、LovyanGFXでビルドします。
Adafruit系ライブラリとタッチスクリーンは使用しません。

このブランチは[pokibon3/cw_decoder4](https://github.com/pokibon3/cw_decoder4)の
`v3.2`（commit `958cf33`）に含まれるDSP・デコーダー改善を、Pico 2へ移植したものです。
Pico 2版の表示バージョンは `3.2-pico2` です。

## 対応ハードウェア

- Raspberry Pi Pico 2（RP2350、150MHzデュアルコア）
- LCD: ST7789 240x320、SPI接続、320x240横表示
- オーディオ入力: GP26 / ADC0、8kHzサンプリング
- 操作: 5個のプッシュスイッチ（GPIOとGND間、内部プルアップ使用）

## 機能

- 欧文／和文モールス信号の復号
- 16列×6行の文字表示（和文カタカナ、濁点・半濁点対応）
- WPM、US/JPモード、選択トーンの表示
- 約300〜1200HzのFFTスペクトラムとピーク周波数表示
- 入力レベルメーター（dBFS）とADCレール到達時の`CLIP`表示
- 生波形、トーンエンベロープ、キー判定を重ねたスコープ表示
- 復号文字を符号区間の時間位置に合わせて流すスコープ・ティッカー
- AUTO同調（500〜1000Hz）と手動600 / 700 / 800 / 900 / 1000Hz

ESP32版v3.2のWi-Fi、OTA、世界時計、セットアップ画面、タッチ操作、
シリアル・スコープログはPico 2版には組み込んでいません。

## 信号処理

- GP26 / ADC0を2面の連結DMAで8kHz連続サンプリング
- 30サンプル（3.75ms）ごとにDSPとデコーダーを更新
- 3ビンfloat Goertzelによる中心・遠サイド・近サイド検出
- 120サンプル窓（通常）／60サンプル窓（高速CW）のWPM追従切り替え
- サイド比、近サイド比、包絡線ジッタ、適応しきい値、ノイズブランカによる誤検出抑制
- 256点FFTによるスペクトラム表示とAUTO同調
- Core 0: LCD描画とスイッチ処理
- Core 1: ADC、Goertzel、FFT、モールス復号

## プッシュスイッチ

| スイッチ | GPIO | Pico 2物理ピン | 機能 |
|---|---:|---:|---|
| SW1 | GP14 | 19 | US / JP切り替え |
| SW2 | GP15 | 20 | トーンを次へ |
| SW3 | GP13 | 17 | KEY表示 ON / OFF |
| SW4 | GP10 | 14 | ENV表示 ON / OFF |
| SW5 | GP11 | 15 | RAW表示 ON / OFF |

SW2の選択順は`AUTO → 600 → 700 → 800 → 900 → 1000Hz → AUTO`です。

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

### オーディオと診断信号

| Pico 2物理ピン | GPIO | 用途 |
|---:|---|---|
| 31 | GP26 / ADC0 | MIC / AF入力 |
| 内蔵 | GP25 | CW状態LED |
| 16 | GP12 | DSP処理タイミングTEST出力 |

配線とAF入力回路は[`hardware/wiring/CW_配線図.pptx`](hardware/wiring/CW_%E9%85%8D%E7%B7%9A%E5%9B%B3.pptx)を参照してください。
GP26は0〜3.3Vの範囲を超えないようにしてください。

## ビルドとPico 2への転送

PlatformIO environmentは`rpipico2`です。

```sh
pio run -e rpipico2
```

転送するファイルは次のUF2です。

```text
.pio/build/rpipico2/firmware.uf2
```

BOOTSELを押しながらPico 2をUSB接続し、表示された`RP2350`ドライブへ
`firmware.uf2`をコピーしてください。

## ソフトウェア構成

- `src/audio.cpp`: RP2350 ADC、連結DMA、クリップ検出
- `src/dsp.cpp`: Goertzel、FFT、AUTO同調、入力レベル、スコープデータ
- `src/decoder.cpp`: v3.2 CW状態機械、ノイズ除去、WPM推定
- `src/decode.cpp`: 欧文／和文モールス符号表
- `src/display.cpp`: ST7789表示、文字バッファ、入力メーター、スコープ
- `src/main.cpp`: デュアルコア起動と5スイッチ操作
- `src/board_config.h`: Pico 2のピン定義

PlatformIOの`build_src_filter`により、上流由来のESP32専用ソースはビルド対象から除外しています。

## 上流更新の取り込み方

このリポジトリでは、Pico 2版を`origin`、元のESP32版を`upstream`として管理します。

```sh
git fetch upstream
git switch -c codex/pico2-next upstream/main
```

新しい上流版ごとにブランチを作り、Pico固有の`audio`、`display`、`main`、
`lgfx_config`、`board_config`を維持しながら、`dsp`、`decoder`、`decode`の変更を
移植して動作確認してからPico 2版のmainへ統合します。上流へ誤ってpushしないよう、
`upstream`のpush URLは無効にして運用します。

## ハードウェア資料

- `hardware/wiring/`: 配線図とLCD寸法資料
- `hardware/case/`: ケースの3MFデータとLCD機械寸法DXF

## ライセンス

モールスデコーダ部分はHjalmar Skovholm Hansen（OZ1JHM）氏のArduino用
CW Decoderを起源とし、GNU General Public License（GPL）が適用されます。
参照できる元ソースと上流リポジトリにはGPLの版番号が明記されていないため、
公開配布前に適用版を確認し、正式な`LICENSE`ファイルを追加してください。

LovyanGFXはGPLではなく、主ライセンスは2条項BSD系です。依存ライブラリの詳細と
同梱ライセンス文は[`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md)を参照してください。

- Original: <http://oz1jhm.dk/content/very-simpel-cw-decoder-easy-build>
- Upstream: <https://github.com/pokibon3/cw_decoder4>
