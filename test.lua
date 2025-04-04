
-- game
function player(alpha,bravo)
    print("incoming player with "..alpha.." and "..bravo)
    --print("incomimg player")
    me.texture = 1
    me.px = 16
    me.py = 16
    me.radius = 16

    local width,height = get_screen_size()
    print(width.."x"..height)

    while true do
        p = collision(enemy)
        if (p) then
            print("ouch!")
            kill(p)
        end

        if buttondown(M_LEFT) then
            create(ball,me.x + 32,me.y)
        end

        if key(K_UP) then
            me.y = me.y - 1
        end

        if key(K_DOWN) then
            me.y = me.y + 1
        end

        frame()
    end
    print("player dead")
end

function ball(x,y)
    me.x = x
    me.y = y
    print("incoming ball")
    me.texture = 0
    me.px = 16
    me.py = 16
    me.radius = 16

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

function enemy(x,y)
    me.x = x
    me.y = y
    print("incoming enemy")
    me.texture = 2
    me.px = 16
    me.py = 16
    me.radius = 16
    
    while true do
    
        p = collision(ball)
        if p then
            kill(p)
            print("bye!")
            break
        end
        
        frame()
    end
end

function main()


    local width,height = get_screen_size()
    load_bank(0,32,32)

    create(player,8,16)

    while true do

        if keydown(K_K) then
            local p = find(ball)
            print("Killing ball")
            kill(p)
        end

        if keydown(K_E) then
            create(enemy,math.random(width),math.random(height))
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
