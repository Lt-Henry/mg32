-- process state
STATE_BORN = 1
STATE_ALIVE = 2
STATE_SLEEP = 3
STATE_DEAD = 4

-- scan codes
K_A = 4
K_B = 5
K_C = 6
K_D = 7
K_E = 8
K_F = 9
K_G = 10
K_H = 11
K_I = 12
K_J = 13
K_K = 14
K_L = 15
K_M = 16
K_N = 17
K_O = 18
K_P = 19
K_Q = 20
K_R = 21
K_S = 22
K_T = 23
K_U = 24
K_V = 25
K_W = 26
K_X = 27
K_Y = 28
K_Z = 29

K_RETURN = 40
K_ESCAPE = 41
K_BACKSPACE = 42
K_TAB = 43
K_SPACE = 44

K_RIGHT = 79
K_LEFT = 80
K_DOWN = 81
K_UP = 82

-- mouse buttons
M_LEFT = 1
M_MIDDLE = 2
M_RIGHT = 3

-- shape
S_POINT = 0
S_CIRCLE = 1
S_BOX = 2

me = nil
_process = {}
_process_frame = {}
_incoming = {}
_id = 32
_last_ticks = 0

function frame()
    if me == nil then

        -- main function

        -- copy current list of process
        _process_frame = {}
        for k,v in pairs(_process) do
            _process_frame[k] = v
        end

        mg32_start_frame()

        local current_ticks = mg32_ticks()
        local ticks = current_ticks - _last_ticks
        _last_ticks = current_ticks

        _tmp = {}

        -- iterate over process
        for k,v in pairs(_process) do
            me = v
            status = coroutine.status(v.thread)
            --print(status)
            if status == "suspended" then
                if v.state == STATE_BORN then
                    v.state = STATE_ALIVE
                    coroutine.resume(v.thread,table.unpack(v.args))
                    mg32_draw_texture(v.bank,v.texture,v.x-v.px,v.y-v.py,v.z)
                    table.insert(_tmp,v)
                elseif v.state == STATE_ALIVE then
                    v.ticks = ticks
                    coroutine.resume(v.thread)
                    if v.angle == 0 then
                        mg32_draw_texture(v.bank,v.texture,v.x-v.px,v.y-v.py,v.z)
                    else
                        mg32_draw_texture_ex(v.bank,v.texture,v.x-v.px,v.y-v.py,v.z,0,v.angle,v.px,v.py)
                    end
                    table.insert(_tmp,v)
                end
            end
        end
        _process = _tmp
        me = nil
        mg32_end_frame()

    else
        coroutine.yield()
    end
end

function create(p,...)

    local cr = coroutine.create(p)

    local child = {
        id = _id,
        fn = p,
        thread = cr,
        state = STATE_BORN,
        args = {...},
        parent = me,

        bank = 0,
        texture = 0,
        x = 0,
        y = 0,
        z = 0,

        -- pivot point
        px = 0,
        py = 0,

        angle = 0,

        shape = S_POINT,
        radius = 0,
        width = 0,
        height = 0,

        ticks= 0

    }

    table.insert(_process, child)
    _id = _id + 1

    return child
end

function kill(p)
    if p then
        p.state = STATE_DEAD
    end
end

function find(p)
    local found = nil

    for k,v in pairs(_process) do
        if v.fn == p then
            found = v
            break
        end
    end

    return found
end

function all(p)
    local tmp = {}

    for k,v in pairs(_process) do
        if v.fn == p and v.state == STATE_ALIVE then
            tmp[k] = v
        end
    end

    return tmp
end

function collision(a)
    local b = me
    local p = nil
    for k,v in pairs(all(a)) do

        if b.shape == S_CIRCLE and v.shape == S_CIRCLE then
            if dist(v,b) < (v.radius + b.radius) then
                p = v
                break
            end
        end

        if b.shape == S_POINT and v.shape == S_CIRCLE then
            if dist(v,b) < v.radius then
                p = v
                break
            end
        end

        if b.shape == S_CIRCLE and v.shape == S_POINT then
            if dist(v,b) < b.radius then
                p = v
                break
            end
        end

        if b.shape == S_POINT and v.shape == S_POINT then
            if dist(v,b) < 1 then
                p = v
                break
            end
        end

        if b.shape == S_POINT and v.shape == S_BOX then
            local wh = v.width/2
            local hh = v.height/2

            local x1 = v.x - wh
            local x2 = v.x + wh
            local y1 = v.y - hh
            local y2 = v.y + hh

            if b.x>= x1 and b.x<=x2 and b.y>=y1 and b.y<=y2 then
                p = v
                break
            end
        end

        if b.shape == S_BOX and v.shape == S_POINT then
            local wh = b.width/2
            local hh = b.height/2

            local x1 = b.x - wh
            local x2 = b.x + wh
            local y1 = b.y - hh
            local y2 = b.y + hh

            if v.x>= x1 and v.x<=x2 and v.y>=y1 and v.y<=y2 then
                p = v
                break
            end
        end

        if b.shape == S_BOX and v.shape == S_CIRCLE then
            local wh = b.width/2
            local hh = b.height/2

            local x1 = b.x - wh
            local x2 = b.x + wh
            local y1 = b.y - hh
            local y2 = b.y + hh

            local testx = v.x
            local testy = v.y

            if v.x < x1 then testx = x1 end
            if v.x > x2 then testx = x2 end
            if v.y < y1 then testy = y1 end
            if v.y > y2 then testy = y2 end

            local distx = v.x - testx
            local disty = v.y - testy

            local distt = math.sqrt( (distx*distx) + (disty*disty))

            if distt < v.radius then
                p = v
                break
            end

        end
    end

    return p
end

function dist(a,b)
    if not a then
        return 0
    end

    if not b then
        b = me
    end

    if not b then
        return 0
    end

    local vx = (a.x  - b.x )
    local vy = (a.y  - b.y )

    return math.sqrt((vx*vx)+(vy*vy))

end

function point_dist(x1,x2,y1,y2)
    local vx = (x1  - x2)
    local vy = (y1  - y2)

    return math.sqrt((vx*vx)+(vy*vy))
end

function advance(steps)

    local sx = steps * math.cos(math.rad(me.angle))
    local sy = steps * math.sin(math.rad(me.angle))
    me.x = me.x + sx
    me.y = me.y + sy
end

function exit()
    mg32_exit()
end

function get_screen_size()
    return mg32_get_screen_size()
end

function draw(texture,x,y,z,bank)
    mg32_draw_texture(bank,texture,x,y,z)
end

function draw_text(txt,x,y,z,bank)
    w,h,tw,th = get_bank_info(bank)

    for n = 1, #txt do
        c = txt:byte(n)

        if (c>31 and c<127) then
            c = c - 32
            mg32_draw_texture(bank,c,x,y,z)
        end
        x = x + tw
    end

end
