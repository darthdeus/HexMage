set wildignore+=bin,*.vcxproj*,*filters,*.sln
nmap <leader>e :VimuxRunCommand("cmake --build build")<CR>
nmap <leader>r :VimuxRunCommand("make -C build run -j")<CR>
