
-- game
function player(alpha,bravo)
    local count = 0
    print("incoming player with "..alpha.." and "..bravo)
    --print("incomimg player")
    while count < 10 do
        print("player "..count)
        count = count + 1
        frame()
    end
    print("player dead")
end

function enemy()
    print("incoming enemy")
    while true do
        print("tick")
        frame()
    end

end

function main()

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

        sleep(500)
        print("frame")
        frame()
    end
end
