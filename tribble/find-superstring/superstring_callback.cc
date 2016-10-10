/*
 Copyright (c) 2016 Jarno Alanko
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see http://www.gnu.org/licenses/ .
 */

#include "superstring_callback.hh"
#include <iostream>
#include <cassert>
#include <algorithm>

Superstring_callback::Superstring_callback(std::size_t n_strings) 
: merges_done(0), n_strings(n_strings), leftend(n_strings), next(n_strings), rightavailable(n_strings){
    for(std::size_t i = 0; i < n_strings; i++){
        leftend[i] = i;
        next[i] = i+1;
        rightavailable[i] = true;
    }
}

bool Superstring_callback::try_merge(std::size_t left_string, std::size_t right_string, std::size_t overlap_length){
    if(leftend[right_string] != left_string){
        merges.push_back(std::make_tuple(left_string, right_string, overlap_length));
        rightavailable[right_string] = false;
        leftend[right_string] = leftend[left_string];
        std::cout << "Merged " << left_string + 1 << " " << right_string + 1 << std::endl;
        return true;
    }
    return false;
}


void Superstring_callback::set_substring_count(std::size_t count)
{
}


bool Superstring_callback::callback(std::size_t read_lex_rank, std::size_t match_length, std::size_t match_sa_begin, std::size_t match_sa_end){
    if(merges_done >= n_strings - 1) return true; // No more merges can be done
    
    std::cout << "CALLBACK " << read_lex_rank << " " << match_length << " " << match_sa_begin << " " << match_sa_end << std::endl;
    assert(read_lex_rank != 0);
    read_lex_rank--; // Change to 0-based indexing
    
    std::size_t k = get_next_right_available(match_sa_begin-1);
    if(k > match_sa_end) {
        // Next one is outside of the suffix array interval, or not found at all
        return false;
    }
    
    if(try_merge(read_lex_rank, k, match_length)) return true;
    
    // Failed, try again a second time
    k = get_next_right_available(k);
    
    if(k > match_sa_end) {
        // Next one is outside of the suffix array interval, or not found at all
        return false;
    }
    
    if(try_merge(read_lex_rank, k, match_length)) return true;
    
    return false; // Should never come here, because the second try should always be succesful
    
}

std::size_t Superstring_callback::get_next_right_available(std::size_t index){
    for(std::size_t i = index+1; i < n_strings; i++){
        if(rightavailable[i]) 
            return i;
    }
    return n_strings;
}

std::string Superstring_callback::build_final_superstring(std::vector<std::string> strings){
    if(strings.size() == 0) return "";
    if(strings.size() == 1) return strings[0];
    
    std::sort(merges.begin(), merges.end());
    std::size_t current_string_idx = n_strings;
    
    // Initilize current_string_idx to the first string in the final superstring
    for(std::size_t i = 0; i < n_strings; i++){
        if(rightavailable[i]){
            current_string_idx = i;
            break;
        }
    }
    assert(current_string_idx != n_strings);
        
    // Concatenate all strings putting in the overlapping region of adjacent strings only once
    std::size_t current_overlap_length;
    std::string superstring;
    for(std::size_t i = 0; i < n_strings - 1; i++){
        std::size_t left_string_idx = std::get<0> (merges[current_string_idx]);
        std::size_t right_string_idx = std::get<1> (merges[current_string_idx]);
        std::size_t overlap = std::get<1> (merges[current_string_idx]);
        
        superstring += strings[left_string_idx].substr(strings[left_string_idx].size() - overlap);
        if(i == n_strings - 2) superstring += strings[right_string_idx]; // Last iteration
        left_string_idx = right_string_idx;
    }
    
    return superstring;
}

