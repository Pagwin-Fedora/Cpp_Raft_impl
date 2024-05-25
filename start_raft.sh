#!/bin/sh
tmux new-session -d bash -c 'build/apps/raft_impl 1 ; sleep 1000' &&
tmux split-window -h build/apps/raft_impl 2 && 
tmux split-window -v build/apps/raft_impl 3 && 
tmux select-pane -t 0 &&
tmux split-window -v build/apps/raft_impl 4 && 
tmux split-window -h build/apps/raft_impl 5&&
tmux attach
