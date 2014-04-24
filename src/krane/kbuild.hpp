/*
Copyright (C) 2013  simplex

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef KRANE_KBUILD_HPP
#define KRANE_KBUILD_HPP

#include "krane_common.hpp"
#include "algebra.hpp"
#include "binary_io_utils.hpp"


namespace Krane {
	class KBuildFile;

	class KBuild : public NestedSerializer<KBuildFile> {
		friend class KBuildFile;
	public:
		typedef computations_float_type float_type;

		class Symbol : public NestedSerializer<KBuild> {
			friend class KBuild;
		public:
			class Frame : public NestedSerializer<Symbol> {
				friend class Symbol;
				friend class KBuild;

			public:
				typedef Triangle<float_type> triangle_type;
				typedef BoundingBox<float_type> bbox_type;

			private:
				std::vector<triangle_type> xyztriangles;
				std::vector<triangle_type> uvwtriangles;

				void setAlphaVertCount(uint32_t n) {
					const uint32_t trigs = n/3;
					xyztriangles.resize(trigs);
					uvwtriangles.resize(trigs);
				}

				/*
				 * Updates the atlas bounding box based on the uvw triangles.
				 */
				void updateAtlasBoundingBox() {
					atlas_bbox.reset();

					size_t ntrigs = uvwtriangles.size();
					for(size_t i = 0; i < ntrigs; i++) {
						atlas_bbox.addPoints(uvwtriangles[i]);
					}

					cropAtlasBoundingBox();
				}

				void cropAtlasBoundingBox() {
					atlas_bbox.intersect(bbox_type(0, 0, 1, 1));
				}

				/*
				 * This is the initial position of this frame as an animation frame.
				 * (i.e., this times the framerate is the initial instant it is shown).
				 *
				 * Counts from 0.
				 */
				uint32_t framenum;

				/*
				 * This is the time length of this frame measured in animation frames.
				 * (i.e., this times the framerate is the time length it is shown)
				 */
				uint32_t duration;

				/*
				 * Corresponds to the x, y, w and h values of the xml.
				 */
				bbox_type bbox;

			public:
				uint32_t getAnimationFrameNumber() const {
					return framenum;
				}

				uint32_t getDuration() const {
					return duration;
				}

				bbox_type& getBoundingBox() {
					return bbox;
				}

				const bbox_type& getBoundingBox() const {
					return bbox;
				}
			
				/*
				 * Bounding box of the corresponding atlas region, in UV coordinates.
				 */
				BoundingBox<float_type> atlas_bbox;

				void addTriangles(const triangle_type& xyz, const triangle_type& uvw) {
					xyztriangles.push_back(xyz);
					uvwtriangles.push_back(uvw);
					atlas_bbox.addPoints(uvw);
					cropAtlasBoundingBox();
				}

				uint32_t countTriangles() const {
					return uint32_t(xyztriangles.size());
				}

				uint32_t countAlphaVerts() const {
					return 3*countTriangles();
				}

				Magick::Image getImage() const;

				void getName(std::string& s) const {
					std::string parent_name;
					parent->getName(parent_name);
					strformat(s, "%s-%u", parent_name.c_str(), (unsigned int)framenum);
				}

				std::string getName() const {
					std::string s;
					getName(s);
					return s;
				}

				void getPath(Compat::Path& p) const {
					parent->getPath(p);
				
					Compat::Path suffix;
					getName(suffix);

					p /= suffix;
					p += ".png";
				}

				Compat::Path getPath() const {
					Compat::Path p;
					getPath(p);
					return p;
				}

				void getUnixPath(Compat::UnixPath& p) const {
					parent->getUnixPath(p);
				
					Compat::UnixPath suffix;
					getName(suffix);

					p /= suffix;
					p += ".png";
				}

				Compat::UnixPath getUnixPath() const {
					Compat::UnixPath p;
					getUnixPath(p);
					return p;
				}

			private:
				std::istream& loadPre(std::istream& in, int verbosity);
				std::istream& loadPost(std::istream& in, int verbosity);
			};

		private:
			std::string name;
			hash_t hash;

			void setHash(hash_t h) {
				hash = h;
			}

		public:
			typedef std::vector<Frame> framelist_t;
			typedef framelist_t::iterator frame_iterator;
			typedef framelist_t::const_iterator frame_const_iterator;

			framelist_t frames;

			void getName(std::string& s) const {
				s = name;
			}

			const std::string& getName() const {
				return name;
			}

			// This counts the number of symbol frames (i.e., distinct
			// animation states).
			uint32_t countFrames() const {
				return frames.size();
			}

			// This counts the number of animation frames (i.e., as a
			// measure of time).
			uint32_t countAnimationFrames() const {
				const size_t nsymframes = frames.size();
				if(nsymframes > 0) {
					const Frame& lastframe = frames[nsymframes - 1];
					return lastframe.framenum + lastframe.duration;
				}
				else {
					return 0;
				}
			}

			void getPath(Compat::Path& p) const {
				p.assign(getName());
			}

			Compat::Path getPath() const {
				Compat::Path p;
				getPath(p);
				return p;
			}

			void getUnixPath(Compat::UnixPath& p) const {
				p.assign(getName());
			}

			Compat::UnixPath getUnixPath() const {
				Compat::UnixPath p;
				getUnixPath(p);
				return p;
			}

			operator const std::string&() const {
				return name;
			}

			uint32_t countAlphaVerts() const {
				uint32_t count = 0;

				const size_t nframes = frames.size();
				for(size_t i = 0; i < nframes; i++) {
					count += frames[i].countAlphaVerts();
				}

				return count;
			}

		private:
			std::istream& loadPre(std::istream& in, int verbosity);
			std::istream& loadPost(std::istream& in, int verbosity);
		};

		typedef Magick::Image atlas_t;

		typedef Symbol::frame_iterator frame_iterator;
		typedef Symbol::frame_const_iterator frame_const_iterator;

	private:
		std::string name;

		std::vector<std::string> atlasnames;
		std::vector<atlas_t> atlases;

	public:
		typedef std::map<hash_t, Symbol> symbolmap_t;
		typedef symbolmap_t::iterator symbolmap_iterator;
		typedef symbolmap_t::const_iterator symbolmap_const_iterator;

		symbolmap_t symbols;


		const std::string& getName() const {
			return name;
		}

		void setName(const std::string& s) {
			name = s;
		}

		const std::string& getFrontAtlasName() const {
			if(atlasnames.empty()) {
				throw(std::logic_error("No atlases."));
			}
			return atlasnames[0];
		}

		atlas_t& getFrontAtlas() {
			if(atlases.empty()) {
				throw(std::logic_error("No atlases."));
			}
			return atlases[0];
		}

		const atlas_t& getFrontAtlas() const {
			if(atlases.empty()) {
				throw(std::logic_error("No atlases."));
			}
			return atlases[0];
		}

		void setFrontAtlas(const atlas_t& a) {
			if(atlases.empty()) {
				atlases.resize(1);
			}
			atlases[0] = a;
		}

		Symbol* getSymbol(hash_t h) {
			symbolmap_t::iterator it = symbols.find(h);
			if(it == symbols.end()) {
				return NULL;
			}
			else {
				return &(it->second);
			}
		}
		
		const Symbol* getSymbol(hash_t h) const {
			symbolmap_t::const_iterator it = symbols.find(h);
			if(it == symbols.end()) {
				return NULL;
			}
			else {
				return &(it->second);
			}
		}

		Symbol* getSymbol(const std::string& symname) {
			return getSymbol(strhash(symname));
		}

		const Symbol* getSymbol(const std::string& symname) const {
			return getSymbol(strhash(symname));
		}

		template<typename OutputIterator>
		void getImages(OutputIterator it) const {
			typedef symbolmap_t::const_iterator symbol_iter;
			typedef Symbol::framelist_t::const_iterator frame_iter;

			for(symbol_iter symit = symbols.begin(); symit != symbols.end(); ++symit) {
				for(frame_iter frit = symit->second.frames.begin(); frit != symit->second.frames.end(); ++frit) {
					*it++ = std::make_pair(frit->getPath(), frit->getImage());
				}
			}
		}

	private:
		std::istream& load(std::istream& in, int verbosity);
	};


	class KBuildFile : public NonCopyable {
	public:
		static const uint32_t MAGIC_NUMBER;

		static const int32_t MIN_VERSION;
		static const int32_t MAX_VERSION;

	private:
		int32_t version;
		KBuild* build;

	public:
		BinIOHelper io;

		static bool checkVersion(int32_t v) {
			return MIN_VERSION <= v && v <= MAX_VERSION;
		}

		bool checkVersion() const {
			return checkVersion(version);
		}

		void versionRequire() const {
			if(!checkVersion()) {
				std::cerr << "Build has unsupported encoding version " << version << "." << std::endl;
				throw(KToolsError("Build has unsupported encoding version."));
			}
		}

		int32_t getVersion() const {
			return version;
		}

		KBuild* getBuild() {
			return build;
		}

		const KBuild* getBuild() const {
			return build;
		}

		std::istream& load(std::istream& in, int verbosity);
		void loadFrom(const std::string& path, int verbosity);

		operator bool() const {
			return build != NULL;
		}

		KBuildFile() : version(MAX_VERSION), build(NULL) {}
	};
}


#endif
