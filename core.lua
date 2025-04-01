
STATE_BORN = 1
STATE_ALIVE = 2
STATE_SLEEP = 3
STATE_DEAD = 4

me = nil
_process = {}
_process_frame = {}
_incoming = {}
_id = 32

_mg32_quit = false

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
                    table.insert(_tmp,v)
                elseif v.state == STATE_ALIVE then
                    coroutine.resume(v.thread)
                    table.insert(_tmp,v)
                end
            end
        end
        _process = _tmp
        me = nil

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
    p.state = STATE_DEAD
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
    _mg32_quit = true
end
