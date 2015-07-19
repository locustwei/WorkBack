--[[
[名称]:操底买入
[说明]:当股票（参数1）探底回升，升幅达（参数2）%，并且股价小于（参数3）元时买入。
[Code]:股票代码
--]]

local p = StockData:new(Code)
return p.Close