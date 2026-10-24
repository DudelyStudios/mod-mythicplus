local frame = CreateFrame("Frame", "MythicPlusFrame", UIParent)
frame:SetSize(260, 110)
frame:SetPoint("TOP", UIParent, "TOP", 0, -40)
frame:SetBackdrop({
    bgFile = "Interface\\DialogFrame\\UI-DialogBox-Background",
    edgeFile = "Interface\\DialogFrame\\UI-DialogBox-Border",
    tile = true,
    tileSize = 32,
    edgeSize = 32,
    insets = { left = 8, right = 8, top = 8, bottom = 8 }
})
frame:Hide()

local text = frame:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
text:SetPoint("TOPLEFT", 16, -16)
text:SetPoint("BOTTOMRIGHT", -16, 16)
text:SetJustifyH("LEFT")
text:SetJustifyV("TOP")

local function Apply(payload)
    local runKey, modifiers, remain, state, held, vault = payload:match("^MythicPlus|run|([^|]*)|([^|]*)|([^|]*)|([^|]*)|([^|]*)|([^|]*)$")
    if not runKey then
        return
    end
    local timer = state == "expired" and "Timer expired" or (remain .. "s left")
    text:SetText(
        "Run key +" .. runKey .. "\n" ..
        modifiers .. "\n" ..
        timer .. "\n" ..
        "Your key +" .. held .. "    Vault +" .. vault
    )
    frame:Show()
end

frame:SetScript("OnEvent", function(_, event, arg1, arg2)
    if event == "CHAT_MSG_ADDON" and arg1 == "MythicPlus" then
        Apply("MythicPlus|" .. (arg2 or ""))
        return
    end
    if event == "CHAT_MSG_WHISPER" and type(arg1) == "string" and arg1:find("^MythicPlus|", 1, true) then
        Apply(arg1)
    end
end)

frame:RegisterEvent("CHAT_MSG_ADDON")
frame:RegisterEvent("CHAT_MSG_WHISPER")
