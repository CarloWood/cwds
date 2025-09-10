define rerun_commands
  set variable debug::Restart<0>::s_restarting = false
  set non-stop off
  finish
  finish
  enable
  set scheduler-locking step
  printf "==> debug::Restart<0>::s_count is now %lu\n", $saved_count
end

define rerun
  set verbose off
  disable

  if $argc > 0
    set $saved_count = (unsigned long)$arg0
  else
    shell rm -f /tmp/gdb.restart.locals.txt
    set logging file /tmp/gdb.restart.locals.txt
    set logging enabled on
    info locals restart_dummy_0
    set logging enabled off
    shell \
if grep -q 'restart_dummy_0' /tmp/gdb.restart.locals.txt; then \
  echo "set \$saved_count = restart_dummy_0.count" > /tmp/gdb.restart.command; \
else \
  echo "set \$saved_count = '::debug::Restart<0>'::s_count._M_i" > /tmp/gdb.restart.command; \
fi

    source /tmp/gdb.restart.command
  end

  printf "*** saved_count is set to %lu.\n", $saved_count

  tbreak debug::Restart<0>::Restart
  commands
    silent
    eval "set variable debug::Restart<0>::s_target_count = %lu", $saved_count
    tbreak debug::Restart<0>::test_break
    commands
      rerun_commands
    end
    cont
  end
  tbreak main
  commands
    silent
    set variable debug::Restart<0>::s_restarting = true
    cont
  end
  set startup-quietly on
  run
end
