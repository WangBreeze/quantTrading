
//@version=6
strategy("闪电战策略", overlay=true, max_labels_count=500, max_lines_count=500, max_boxes_count=500, initial_capital=10000, default_qty_type=strategy.percent_of_equity, default_qty_value=10)

// ===== 参数设置 =====
a     = input.float(2,  "UT Key Value")
c     = input.int(4,   "UT ATR Period")
useHA = input.bool(false, "UT Signals from Heikin Ashi")

lenSTC      = input.int(80,  'STC Length')
fastLenSTC  = input.int(27,  'STC FastLength')
slowLenSTC  = input.int(50,  'STC SlowLength')
alphaSTC    = input.float(0.5, 'STC Smoothing')

riskATRmult = input.float(0.4, "止损ATR倍数", step=0.1)
rrRatio     = input.float(2.5, "风险回报比", step=0.1)

// ===== UT Bot =====
srcUT = useHA ? request.security(ticker.heikinashi(syminfo.tickerid), timeframe.period, close) : close
xATR  = ta.atr(c)
nLoss = a * xATR

var float xATRTrailingStop = na
xATRTrailingStop := if srcUT > nz(xATRTrailingStop[1], 0) and srcUT[1] > nz(xATRTrailingStop[1], 0)
    math.max(nz(xATRTrailingStop[1]), srcUT - nLoss)
else if srcUT < nz(xATRTrailingStop[1], 0) and srcUT[1] < nz(xATRTrailingStop[1], 0)
    math.min(nz(xATRTrailingStop[1]), srcUT + nLoss)
else if srcUT > nz(xATRTrailingStop[1], 0)
    srcUT - nLoss
else
    srcUT + nLoss

emaUT = ta.ema(srcUT, 1)
above = ta.crossover(emaUT, xATRTrailingStop)
below = ta.crossover(xATRTrailingStop, emaUT)

longSignal  = srcUT > xATRTrailingStop and above
shortSignal = srcUT < xATRTrailingStop and below

// ===== Hull Suite =====
srcHull         = input.source(close, title="Hull Source")
modeSwitch      = input.string("Hma", title="Hull Variation", options=["Hma", "Thma", "Ehma"])
lengthHull      = input.int(40, title="Hull Length")
lengthMult      = input.float(1.0, title="Hull Length Multiplier")

HMA(_src, _length)  => ta.wma(2 * ta.wma(_src, _length / 2) - ta.wma(_src, _length), math.round(math.sqrt(_length)))
EHMA(_src, _length) => ta.ema(2 * ta.ema(_src, _length / 2) - ta.ema(_src, _length), math.round(math.sqrt(_length)))
THMA(_src, _length) => ta.wma(ta.wma(_src,_length / 3) * 3 - ta.wma(_src, _length / 2) - ta.wma(_src, _length), _length)

Mode(modeSwitch, src, len) =>
      modeSwitch == "Hma"  ? HMA(src, len) :
      modeSwitch == "Ehma" ? EHMA(src, len) :
      modeSwitch == "Thma" ? THMA(src, len/2) : na

MHULL = Mode(modeSwitch, srcHull, int(lengthHull * lengthMult))

// ===== STC计算模块 =====
stcMACD(src, fast, slow) =>
    fastMA = ta.ema(src, fast)
    slowMA = ta.ema(src, slow)
    fastMA - slowMA

stcCalc(len, fast, slow, alpha) =>
    var stc1 = 0.0
    var stc2 = 0.0
    var stc3 = 0.0
    var stc4 = 0.0
    macdVal = stcMACD(close, fast, slow)
    ll  = ta.lowest(macdVal, len)
    hh  = ta.highest(macdVal, len) - ll
    stc1 := hh > 0 ? (macdVal - ll) / hh * 100 : nz(stc1[1])
    stc2 := na(stc2[1]) ? stc1 : stc2[1] + alpha * (stc1 - stc2[1])
    ll2 = ta.lowest(stc2, len)
    hh2 = ta.highest(stc2, len) - ll2
    stc3 := hh2 > 0 ? (stc2 - ll2) / hh2 * 100 : nz(stc3[1])
    stc4 := na(stc4[1]) ? stc3 : stc4[1] + alpha * (stc3 - stc4[1])
    stc4

stcVal = stcCalc(lenSTC, fastLenSTC, slowLenSTC, alphaSTC)

// ===== 止损止盈计算 =====
longStop  = low  - xATR * riskATRmult
longTP    = close + (close - longStop) * rrRatio
shortStop = high + xATR * riskATRmult
shortTP   = close - (shortStop - close) * rrRatio

// ===== 方块尺寸（高度为ATR的5%，宽度为1根柱子） =====
boxHalfH = xATR * 0.05

// ===== R值计算 =====
longR  = (longTP - close) / (close - longStop)
shortR = (close - shortTP) / (shortStop - close)

// ===== 有效信号（R≥rrRatio） =====
validLong  = longSignal  and close > MHULL and stcVal > 25 and close > open and longR >= rrRatio
validShort = shortSignal and close < MHULL and stcVal > 75 and close < open and shortR >= rrRatio

// ===== 交易ID生成 =====
var int tradeID = 0
if (validLong or validShort)
    tradeID += 1

// ===== 策略下单 =====
if validLong
    strategy.entry("多#" + str.tostring(tradeID), strategy.long)
    strategy.exit("多平仓#" + str.tostring(tradeID), "多#" + str.tostring(tradeID), stop=longStop, limit=longTP)
    // 绘制多单：上方绿色TP方块，下方红色SL方块（位于入场K线）
    box.new(left=bar_index, top=longTP + boxHalfH, right=bar_index + 1, bottom=longTP - boxHalfH, xloc=xloc.bar_index, bgcolor=color.new(color.green, 0), border_color=color.new(color.green, 0))
    box.new(left=bar_index, top=longStop + boxHalfH, right=bar_index + 1, bottom=longStop - boxHalfH, xloc=xloc.bar_index, bgcolor=color.new(color.red, 0),   border_color=color.new(color.red, 0))

if validShort
    strategy.entry("空#" + str.tostring(tradeID), strategy.short)
    strategy.exit("空平仓#" + str.tostring(tradeID), "空#" + str.tostring(tradeID), stop=shortStop, limit=shortTP)
    // 绘制空单：上方红色SL方块，下方绿色TP方块（位于入场K线）
    box.new(left=bar_index, top=shortStop + boxHalfH, right=bar_index + 1, bottom=shortStop - boxHalfH, xloc=xloc.bar_index, bgcolor=color.new(color.red, 0),   border_color=color.new(color.red, 0))
    box.new(left=bar_index, top=shortTP + boxHalfH,   right=bar_index + 1, bottom=shortTP - boxHalfH,   xloc=xloc.bar_index, bgcolor=color.new(color.green, 0), border_color=color.new(color.green, 0))

// ====== ① 移动止盈：浮盈≥1 R 后把止损移到开仓价+0.1 ATR ======
if strategy.position_size != 0
    var float entryPrice = strategy.position_avg_price
    float openPL = strategy.position_size > 0 ? (close - entryPrice) / (entryPrice - strategy.position_avg_price) : (entryPrice - close) / (strategy.position_avg_price - entryPrice)
    float trailDist = xATR * 0.1
    if openPL >= 1
        if strategy.position_size > 0
            strategy.exit("LongTrail", from_entry="多#"+str.tostring(tradeID), stop=entryPrice + trailDist, limit=longTP)
        else
            strategy.exit("ShortTrail", from_entry="空#"+str.tostring(tradeID), stop=entryPrice - trailDist, limit=shortTP)

// ====== ② 时段过滤：仅 UTC 00:00-16:00 开新仓 ======
bool inSession = hour(time) >= 0 and hour(time) < 16
validLong  := validLong  and inSession
validShort := validShort and inSession

// ===== 信号箭头（固定文字） =====
plotshape(validLong,  title="多信号", text="多", style=shape.labelup,   location=location.belowbar, color=color.green, textcolor=color.white, size=size.tiny)
plotshape(validShort, title="空信号", text="空", style=shape.labeldown, location=location.abovebar, color=color.red,   textcolor=color.white, size=size.tiny)