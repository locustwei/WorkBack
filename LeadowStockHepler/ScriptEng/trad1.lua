--[[
[名称]:操底买入
[说明]:当股票（参数1）探底回升，升幅达（参数2）%，并且股价小于（参数3）元时买入。
[Symbol]:股票代码
[nPercent]:回升幅度（百分比%）
[nValue]:不高于（元）
[nVolumn]:买入数（手）
--]]


if Symbol==nil then
	return "Symbol = nil"
end

local p = StockData:new(Symbol)

if p.Low == 0 then
	return "p.Low = 0"
end

local n = (p.Current - p.Low) / p.Low        --低价回升百分比

if (p.Current < nValue) and (n > nPercent) then
	return p:Buy(nVolumn)                    --买入（手）
else
	return "条件不成立"
end