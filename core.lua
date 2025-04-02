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

K_ESCAPE = 41

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
                    mg32_draw_texture(v.bank,v.texture,v.x,v.y)
                    table.insert(_tmp,v)
                elseif v.state == STATE_ALIVE then
                    coroutine.resume(v.thread)
                    mg32_draw_texture(v.bank,v.texture,v.x,v.y)
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

function exit()
    mg32_exit()
end

function get_screen_size()
    return mg32_get_screen_size()
end
