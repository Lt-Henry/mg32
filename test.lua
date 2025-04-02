
-- game
function player(alpha,bravo)
    print("incoming player with "..alpha.." and "..bravo)
    --print("incomimg player")
    while me.y < 480 do

        if key(KEY_Z) then
            create(enemy)
        end

        me.y = me.y + 1
        frame()
    end
    print("player dead")
end

function enemy()
    print("incoming enemy")
    me.texture = 1
    while true do
        --print("tick")
        me.x = me.x + 1
        frame()
    end

end

function main()

    load_bank(0,32,32)

    create(player,8,16)
    local p = find(player)
    print(p.id)

    local e = create(enemy)
    local count = 0
    while true do
        count = count + 1
        if (count == 30) then
            print("Killing enemy")
            kill(e)
        end

        if key(KEY_ESCAPE) then
            print("Exit")
            exit()
        end

        sleep(15)
        --print("frame")
        frame()
    end
end
