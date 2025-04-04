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

function frame()
    if me == nil then

        -- main function

        -- copy current list of process
        _process_frame = {}
        for k,v in pairs(_process) do
            _process_frame[k] = v
        end

        mg32_start_frame()

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
                    mg32_draw_texture(v.bank,v.texture,v.x-v.px,v.y-v.py)
                    table.insert(_tmp,v)
                elseif v.state == STATE_ALIVE then
                    coroutine.resume(v.thread)
                    mg32_draw_texture(v.bank,v.texture,v.x-v.px,v.y-v.py)
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

        shape = S_POINT,
        radius = 0,
        width = 0,
        height = 0

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
        if v.fn == p then
            tmp[k] = v
        end
    end

    return tmp
end

function collision(a)
    local b = me
    local p = nil
    for k,v in pairs(all(a)) do

        if dist(v,b) < (v.radius + b.radius) then
            p = v
            break
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

function exit()
    mg32_exit()
end

function get_screen_size()
    return mg32_get_screen_size()
end
