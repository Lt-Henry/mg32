
-- game
function player(alpha,bravo)
    print("incoming player with "..alpha.." and "..bravo)
    --print("incomimg player")
    me.texture = 1

    local width,height = get_screen_size()
    print(width.."x"..height)
    local vy = 1

    while true do

        if keydown(K_Z) then
            create(enemy,me.x,me.y)
        end

        me.y = me.y + vy

        if me.y>height then
            vy = -1
        end

        if me.y<0 then
            vy = 1
        end

        frame()
    end
    print("player dead")
end

function enemy(x,y)
    me.x = x
    me.y = y
    print("incoming enemy")
    me.texture = 0

    local width,height = get_screen_size()
    local vx = 4
    while true do
        --print("tick")
        me.x = me.x + vx
        if me.x > width then
            vx = -4
        end

        if me.x < 0 then
            vx = 4
        end
        frame()
    end

end

function main()


    load_bank(0,32,32)

    create(player,8,16)

    while true do

        if keydown(K_K) then
            local p = find(enemy)
            print("Killing enemy")
            kill(p)
        end

        if key(K_ESCAPE) then
            print("Exit")
            exit()
        end

        sleep(15)
        --print("frame")
        frame()
    end
end
