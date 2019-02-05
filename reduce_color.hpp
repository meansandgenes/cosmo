#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <libgen.h>
#include <boost/algorithm/string.hpp>
#include "sdsl/sd_vector.hpp"
#include <sdsl/bit_vectors.hpp>
#include <sdsl/wavelet_trees.hpp>
#include <bits/stdc++.h>
#include <cstdio>
#include <cstdlib>
using namespace std;
using namespace sdsl;

class reduction {

public:
  int num_color = 0;
  map<size_t,std::vector<size_t>> color_map;
  vector<size_t> build_backup;



  std::map<string,size_t> index_maker(){
    cerr<<"Making the pair map"<<endl;
    cerr<<"================================================="<<endl;
    ifstream f ("pair" , std::ifstream::in);
    map<string,size_t> pairs;
    string s;
    size_t index = 0;
    while( getline(f,s) ){
      locale loc;
      string ss = "";
      for (string::size_type i = 0; i < s.length(); ++i)
        ss += toupper(s[i],loc);
        pairs.insert(pair<string,size_t>(ss,index));
        index++;
      }
      map<string,size_t>::iterator it = pairs.begin();
      return pairs;
    }


    map<string,size_t> pairs = index_maker();
    vector<int> online_kmers{vector<int>(pairs.size(),-1)};




    vector<string> kmer_counter_read(size_t k, string read)
    {
      string kmer;
      vector<string> found_kmers_per_read;
      size_t pos = 0;
      while(pos != read.size()-k+1){
        kmer = read.substr(pos,k);
        map<string,size_t>::iterator find = pairs.find(kmer);
        if (find != pairs.end())
          found_kmers_per_read.push_back(kmer);

          pos++;
        }
      return found_kmers_per_read;
    }


    int which_color ( std::vector<string> &subreadit){
      int max = -1;
      for (size_t i = 0; i < subreadit.size(); ++i){
        if (online_kmers[pairs[subreadit[i]]] > max )
          max = online_kmers[pairs[subreadit[i]]];
    }
      for (size_t i = 0; i < subreadit.size(); ++i){
        online_kmers[pairs[subreadit[i]]] += 1;
      }
      return max+1;
    }


    void build (vector<string> found_kmers_per_read){
      int color = which_color(found_kmers_per_read);
      if (color < num_color ){
        for (vector<string>::iterator it = found_kmers_per_read.begin(); it!= found_kmers_per_read.end(); ++it)
          build_backup.push_back(pairs[*it] + color * pairs.size());
            }
      else{
        num_color++;
        for (vector<string>::iterator it = found_kmers_per_read.begin(); it!= found_kmers_per_read.end(); ++it)
          build_backup.push_back(pairs[*it] + color * pairs.size());//}
          }
      }


    void parser(ifstream& f , int num_reads)
    {
      size_t count = 0;
      string s;

      if (!getline(f, s) || !(s[0] == '>'||s[0] == '@')) {
          cerr << "Make sure that the input is fastq file" << std::endl;
          exit(EXIT_FAILURE);
        }
    bool mode = true;
    s = "";
    while(getline(f, s) && s.size() > 0){
      string read;
      if (s[0] == '>' || s[0] == '@') {
        mode = true;
        s = "";
        }

      else if (s[0] == '+') {
        mode = false;
        }

      else if (mode == true) {
        if (count % 10000 == 0) cout<<"Processing read number "<< count <<
        " ("<<(double)count * 100/ num_reads<<" % is processed)\n";
        count++;
        read += s;
        vector<string> found_kmers_per_read = kmer_counter_read(32, read);
        build (found_kmers_per_read);
        }
    	}
    }


    void build_recolored_matrix (ifstream& f , int num_reads){
      parser(f, num_reads);
      size_t n = (num_color ) * pairs.size();
      size_t m = build_backup.size();
      cerr<<"================================================="<<endl;
      cerr << "Matrix dimensions: n = " << n<< " m = " << m <<endl;
      sdsl::sd_vector_builder b_builder(n, m);
      std::sort(build_backup.begin(), build_backup.end());
      for(vector<size_t>::iterator it = build_backup.begin(); it!= build_backup.end(); ++it){
      	b_builder.set(*it);
      }

      sdsl::sd_vector<> b(b_builder);
      cerr << "Total size of reduced matrix: "<<size_in_mega_bytes(b) <<" Mb"<<endl;
      cerr<<"Writing the matrix to output_matrix_reduced"<<endl;
      string outfilename = "output_matrix_reduced";
      sdsl::store_to_file(b, outfilename);

      }

};
