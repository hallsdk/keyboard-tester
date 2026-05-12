// ---------- state ----------
const state = {
  token: localStorage.getItem("ktester_token") || "",
  me:    null,
  factories: null,   // cached list of {id,name}
};

async function loadFactories(force = false) {
  if (state.factories && !force) return state.factories;
  state.factories = await api("/api/factories");
  return state.factories;
}
function factoryName(id) {
  if (!state.factories) return String(id ?? "");
  const f = state.factories.find(x => x.id === id);
  return f ? f.name : ("#" + id);
}

// ---------- helpers ----------
async function api(path, { method = "GET", body = null, raw = false } = {}) {
  const headers = {};
  if (state.token) headers["Authorization"] = "Bearer " + state.token;
  if (body && !raw) headers["Content-Type"] = "application/json";
  const res = await fetch(path, {
    method,
    headers,
    body: body == null ? undefined : (raw ? body : JSON.stringify(body)),
  });
  if (res.status === 401) {
    logout();
    throw new Error("登录已失效，请重新登录");
  }
  if (!res.ok) {
    let msg = res.statusText;
    try { const j = await res.json(); msg = j.error || msg; } catch {}
    throw new Error(msg);
  }
  if (raw) return res;
  if (res.status === 204) return null;
  const ct = res.headers.get("Content-Type") || "";
  return ct.includes("application/json") ? res.json() : res.text();
}

function toast(msg, isErr = false) {
  let t = document.querySelector(".toast");
  if (!t) {
    t = document.createElement("div"); t.className = "toast";
    document.body.appendChild(t);
  }
  t.textContent = msg;
  t.classList.toggle("err", isErr);
  t.classList.add("show");
  clearTimeout(t._h); t._h = setTimeout(() => t.classList.remove("show"), 2200);
}

function el(html) {
  const t = document.createElement("template");
  t.innerHTML = html.trim();
  return t.content.firstChild;
}

function logout() {
  state.token = ""; state.me = null;
  localStorage.removeItem("ktester_token");
  location.hash = "#/login";
  render();
}

function roleLabel(r) {
  if (r === "super_admin") return `<span class="role-pill super">超级管理员</span>`;
  if (r === "admin")       return `<span class="role-pill admin">管理员</span>`;
  return `<span class="role-pill user">用户</span>`;
}

function escapeHtml(s) {
  if (s == null) return "";
  return String(s)
    .replaceAll("&", "&amp;").replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;").replaceAll('"', "&quot;");
}

// ---------- routing ----------
function currentRoute() {
  const h = location.hash || "#/devices";
  return h.replace(/^#/, "");
}

window.addEventListener("hashchange", render);

async function render() {
  const view = document.getElementById("view");
  const nav  = document.getElementById("nav");
  const ua   = document.getElementById("userArea");
  view.innerHTML = ""; nav.innerHTML = ""; ua.innerHTML = "";

  // Need login?
  if (!state.token) {
    location.hash.startsWith("#/register") ? renderRegister(view) : renderLogin(view);
    return;
  }

  // Ensure me
  if (!state.me) {
    try { state.me = await api("/api/auth/me"); }
    catch { return logout(); }
  }

  // Top user area
  ua.innerHTML = `${escapeHtml(state.me.username)} ${roleLabel(state.me.role)} ` +
    `<button class="ghost" id="btnPw">修改密码</button>` +
    `<button class="ghost" id="btnLogout">退出</button>`;
  ua.querySelector("#btnLogout").onclick = logout;
  ua.querySelector("#btnPw").onclick = () => location.hash = "#/password";

  // Nav
  const tabs = [
    { id: "devices",   label: "设备/配列", roles: ["user","admin","super_admin"] },
    { id: "users",     label: "用户管理", roles: ["admin","super_admin"] },
    { id: "factories", label: "工厂管理", roles: ["super_admin"] },
  ];
  const cur = currentRoute();
  for (const t of tabs) {
    if (!t.roles.includes(state.me.role)) continue;
    const b = document.createElement("button");
    b.textContent = t.label;
    if (cur.startsWith("/" + t.id)) b.classList.add("active");
    b.onclick = () => location.hash = "#/" + t.id;
    nav.appendChild(b);
  }

  // Route
  const r = currentRoute();
  if (r.startsWith("/devices"))   return renderDevices(view);
  if (r.startsWith("/users"))     return renderUsers(view);
  if (r.startsWith("/factories")) return renderFactories(view);
  if (r.startsWith("/password"))  return renderPassword(view);
  location.hash = "#/devices";
}

// ---------- views ----------
function renderLogin(view) {
  const box = el(`
    <div class="auth-box">
      <h1>登录</h1>
      <div class="tabs">
        <button class="active" id="tabL">登录</button>
        <button id="tabR">注册</button>
      </div>
      <label>用户名</label><input id="u" autocomplete="username" />
      <label>密码</label><input id="p" type="password" autocomplete="current-password" />
      <div style="margin-top:14px"><button id="ok" style="width:100%">登录</button></div>
    </div>`);
  view.appendChild(box);
  box.querySelector("#tabR").onclick = () => location.hash = "#/register";
  box.querySelector("#ok").onclick = async () => {
    try {
      const j = await api("/api/auth/login", {
        method: "POST",
        body: { username: box.querySelector("#u").value, password: box.querySelector("#p").value },
      });
      state.token = j.token;
      localStorage.setItem("ktester_token", j.token);
      state.me = null;
      location.hash = "#/devices";
    } catch (e) { toast(e.message, true); }
  };
}

function renderRegister(view) {
  const box = el(`
    <div class="auth-box">
      <h1>注册</h1>
      <div class="tabs">
        <button id="tabL">登录</button>
        <button class="active" id="tabR">注册</button>
      </div>
      <label>用户名 (3-32)</label><input id="u" />
      <label>密码 (≥6)</label><input id="p" type="password" />
      <label>选择所属工厂</label>
      <select id="fac"><option value="">加载中...</option></select>
      <div style="margin-top:14px"><button id="ok" style="width:100%">提交注册申请</button></div>
      <p class="muted" style="text-align:center; margin-top:10px">
        注册后需等待工厂管理员审核激活。
      </p>
      <div id="pending-msg" style="display:none;text-align:center;padding:12px 0">
        <b>注册申请已提交！</b><br>
        <span class="muted">请等待管理员审核，审核通过后即可登录。</span><br><br>
        <button id="go-login" class="ghost">返回登录</button>
      </div>
    </div>`);
  view.appendChild(box);
  box.querySelector("#tabL").onclick = () => location.hash = "#/login";
  // Load factory list.
  (async () => {
    try {
      const facs = await api("/api/factories/public");
      const sel = box.querySelector("#fac");
      sel.innerHTML = `<option value="">-- 请选择工厂 --</option>` +
        (facs || []).map(f => `<option value="${f.id}">${escapeHtml(f.name)}</option>`).join("");
    } catch (e) { box.querySelector("#fac").innerHTML = `<option value="">（加载失败）</option>`; }
  })();
  box.querySelector("#ok").onclick = async () => {
    const fid = parseInt(box.querySelector("#fac").value, 10);
    if (!fid) { toast("请选择工厂", true); return; }
    try {
      await api("/api/auth/register", {
        method: "POST",
        body: { username: box.querySelector("#u").value, password: box.querySelector("#p").value, factory_id: fid },
      });
      box.querySelector("#ok").style.display = "none";
      box.querySelector("#pending-msg").style.display = "";
      box.querySelector("#go-login").onclick = () => { location.hash = "#/login"; };
    } catch (e) { toast(e.message, true); }
  };
}

function renderPassword(view) {
  const box = el(`
    <div class="card" style="max-width:480px; margin:0 auto">
      <h2>修改密码</h2>
      <label>当前密码</label><input id="o" type="password" />
      <label>新密码 (≥6)</label><input id="n" type="password" />
      <div style="margin-top:14px"><button id="ok">提交</button></div>
    </div>`);
  view.appendChild(box);
  box.querySelector("#ok").onclick = async () => {
    try {
      await api("/api/auth/change-password", {
        method: "POST",
        body: {
          old_password: box.querySelector("#o").value,
          new_password: box.querySelector("#n").value,
        },
      });
      toast("已修改");
      location.hash = "#/devices";
    } catch (e) { toast(e.message, true); }
  };
}

// ---------- devices ----------
async function renderDevices(view) {
  const canEdit = state.me.role === "admin" || state.me.role === "super_admin";
  view.innerHTML = `<div class="card"><h2>设备 / 配列</h2>
    <div id="list">加载中...</div>
    ${canEdit ? `<div style="margin-top:14px"><button id="add">+ 新增设备</button></div>` : ""}
  </div>`;
  if (canEdit) view.querySelector("#add").onclick = () => editDeviceDialog(null);
  try {
    await loadFactories();
    const list = await api("/api/devices");
    const tbl = `
      <table>
        <thead><tr>
          <th>工厂</th><th>VID-PID</th><th>名称</th><th>说明</th>
          <th>配列文件</th><th>更新时间</th>
          ${canEdit ? `<th>操作</th>` : ""}
        </tr></thead>
        <tbody>
          ${(list||[]).map(d => `
            <tr>
              <td>${escapeHtml(factoryName(d.factory_id))}</td>
              <td><code>${escapeHtml(d.vid)}-${escapeHtml(d.pid)}</code></td>
              <td>${escapeHtml(d.name)}</td>
              <td>${escapeHtml(d.description || "")}</td>
              <td><a href="/api/devices/${d.vid}-${d.pid}/layout?token=${encodeURIComponent(state.token)}">${escapeHtml(d.layout_file)}</a></td>
              <td class="muted">${d.updated_at?.slice(0,19).replace("T"," ") || ""}</td>
              ${canEdit ? `<td class="actions">
                <button class="ghost" data-edit="${d.id}">编辑</button>
                <button class="danger" data-del="${d.id}" data-name="${escapeHtml(d.vid+'-'+d.pid)}">删除</button>
              </td>` : ""}
            </tr>`).join("")}
        </tbody>
      </table>`;
    view.querySelector("#list").innerHTML = list && list.length ? tbl : "<p class='muted'>暂无设备</p>";
    if (canEdit) {
      view.querySelectorAll("[data-edit]").forEach(b => b.onclick = () => {
        const d = list.find(x => x.id == b.dataset.edit); editDeviceDialog(d);
      });
      view.querySelectorAll("[data-del]").forEach(b => b.onclick = async () => {
        if (!confirm(`确定删除 ${b.dataset.name}？此操作不可恢复。`)) return;
        try { await api("/api/devices/" + b.dataset.del, { method: "DELETE" }); toast("已删除"); renderDevices(view); }
        catch (e) { toast(e.message, true); }
      });
    }
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

function editDeviceDialog(existing) {
  const isNew = !existing;
  const view = document.getElementById("view");
  const facOpts = (state.factories || [])
    .map(f => `<option value="${f.id}">${escapeHtml(f.name)}</option>`).join("");
  const canChangeFactory = state.me.role === "super_admin" || isNew;
  view.innerHTML = `
    <div class="card">
      <h2>${isNew ? "新增设备" : "编辑设备 #" + existing.id}</h2>
      <label>所属工厂</label>
      <select id="fac" ${canChangeFactory ? "" : "disabled"}>${facOpts}</select>
      <div class="row">
        <div><label>VID (0xHHHH)</label>
          <input id="vid" value="${existing ? escapeHtml(existing.vid) : "0x"}" ${existing ? "readonly" : ""}/>
        </div>
        <div><label>PID (0xHHHH)</label>
          <input id="pid" value="${existing ? escapeHtml(existing.pid) : "0x"}" ${existing ? "readonly" : ""}/>
        </div>
      </div>
      <label>设备名称</label><input id="name" value="${existing ? escapeHtml(existing.name) : ""}"/>
      <label>说明</label><input id="desc" value="${existing ? escapeHtml(existing.description||"") : ""}"/>
      <label>配列文件名 (如 wp75.c / xp75.json，可含子目录)</label>
      <input id="file" value="${existing ? escapeHtml(existing.layout_file) : ""}"/>
      <label>配列内容 (留空则不修改文件，仅改元数据)</label>
      <textarea id="body" placeholder="粘贴 .c 或 .json 内容"></textarea>
      <div style="margin-top:14px" class="actions">
        <button id="ok">${isNew ? "创建" : "保存"}</button>
        <button class="ghost" id="cancel">取消</button>
      </div>
    </div>`;
  if (existing && existing.factory_id) view.querySelector("#fac").value = existing.factory_id;
  // Auto-fill layout filename from device name when field is empty.
  if (isNew) {
    const nameEl = view.querySelector("#name");
    const fileEl = view.querySelector("#file");
    nameEl.addEventListener("input", () => {
      const slug = nameEl.value.trim().toLowerCase().replace(/\s+/g, "_").replace(/[^a-z0-9_\-]/g, "");
      if (slug) fileEl.value = slug + ".c";
      else fileEl.value = "";
    });
  }
  view.querySelector("#cancel").onclick = () => { renderDevices(view); };
  view.querySelector("#ok").onclick = async () => {
    const payload = {
      name: view.querySelector("#name").value,
      description: view.querySelector("#desc").value,
      layout_file: view.querySelector("#file").value,
      layout_body: view.querySelector("#body").value,
    };
    if (canChangeFactory) payload.factory_id = parseInt(view.querySelector("#fac").value, 10);
    try {
      if (isNew) {
        payload.vid = view.querySelector("#vid").value;
        payload.pid = view.querySelector("#pid").value;
        if (!payload.layout_body) { toast("新建时必须粘贴配列内容", true); return; }
        await api("/api/devices", { method: "POST", body: payload });
      } else {
        await api("/api/devices/" + existing.id, { method: "PUT", body: payload });
      }
      await renderDevices(view);
      toast("已保存");
    } catch (e) { toast(e.message, true); }
  };
}

// ---------- users ----------
async function renderUsers(view) {
  const isSuper = state.me.role === "super_admin";
  const isAdmin = state.me.role === "admin" || isSuper;
  view.innerHTML = `<div class="card">
    <h2>用户管理</h2>
    <div id="list">加载中...</div>
    <div style="margin-top:14px" class="actions">
      ${isSuper ? `<button id="add">+ 新增用户</button>` : ""}
      ${isAdmin ? `<button id="addEmp" class="ghost">+ 新增员工</button>` : ""}
    </div>
  </div>
  <div class="card" style="margin-top:18px">
    <h2>工厂可见设备</h2>
    <p class="muted" style="margin:4px 0 10px">统一设置该工厂员工可以看到哪些设备（不选则全部可见）</p>
    <div id="fac-dev-list">加载中...</div>
  </div>
  <div class="card" style="margin-top:18px">
    <h2>待审核注册 <span id="pending-count" class="muted"></span></h2>
    <div id="pending-list">加载中...</div>
  </div>`;
  if (isSuper) view.querySelector("#add").onclick = () => editUserDialog(null);
  if (isAdmin) view.querySelector("#addEmp").onclick = () => editEmployeeDialog();
  try {
    await loadFactories();
    const [list, pending] = await Promise.all([
      api("/api/users"),
      api("/api/users/pending"),
    ]);
    const tbl = `
      <table>
        <thead><tr>
          <th>ID</th><th>用户名</th><th>角色</th>
          <th>工厂</th>
          <th>创建</th><th>最近登录</th>
          <th>操作</th>
        </tr></thead>
        <tbody>
          ${(list||[]).map(u => `
            <tr>
              <td>${u.id}</td>
              <td><b>${escapeHtml(u.username)}</b></td>
              <td>${roleLabel(u.role)}</td>
              <td>${(u.factory_ids||[]).map(id => escapeHtml(factoryName(id))).join(", ") || (u.role==="super_admin" ? "<span class='muted'>全部</span>" : "")}</td>
              <td class="muted">${(u.created_at||"").slice(0,19).replace("T"," ")}</td>
              <td class="muted">${(u.last_login||"").slice(0,19).replace("T"," ")}</td>
              <td class="actions">
                ${isSuper ? `<button class="ghost" data-edit="${u.id}">编辑</button>` : ""}
                ${(isSuper && u.id !== state.me.id) ? `<button class="danger" data-del="${u.id}" data-name="${escapeHtml(u.username)}">删除</button>` : ""}
              </td>
            </tr>`).join("")}
        </tbody>
      </table>
    `;
    view.querySelector("#list").innerHTML = tbl;
    view.querySelectorAll("[data-edit]").forEach(b => b.onclick = () => {
      const u = list.find(x => x.id == b.dataset.edit); editUserDialog(u);
    });
    view.querySelectorAll("[data-del]").forEach(b => b.onclick = async () => {
      if (!confirm(`确定删除用户 ${b.dataset.name}？`)) return;
      try { await api("/api/users/" + b.dataset.del, { method: "DELETE" }); toast("已删除"); renderUsers(view); }
      catch (e) { toast(e.message, true); }
    });
    // Factory visible devices section — one block per factory admin manages.
    const myFacs = state.factories || [];
    if (myFacs.length === 0) {
      view.querySelector("#fac-dev-list").innerHTML = `<p class="muted">暂无工厂</p>`;
    } else {
      const facDevHtml = myFacs.map(f =>
        `<div style="margin-bottom:10px">
          <b>${escapeHtml(f.name)}</b>
          <button class="ghost" style="margin-left:8px" data-fac-dev="${f.id}">设置可见设备</button>
          <span class="muted" id="fac-dev-summary-${f.id}"></span>
        </div>`
      ).join("");
      view.querySelector("#fac-dev-list").innerHTML = facDevHtml;
      // Load summaries.
      for (const f of myFacs) {
        api(`/api/factories/${f.id}/devices`).then(j => {
          const el = view.querySelector(`#fac-dev-summary-${f.id}`);
          if (el) el.textContent = (j.device_ids||[]).length === 0 ? "（全部可见）" : `（已限制 ${j.device_ids.length} 个）`;
        }).catch(() => {});
      }
      view.querySelectorAll("[data-fac-dev]").forEach(b => b.onclick = () => {
        const f = myFacs.find(x => x.id == b.dataset.facDev); editFactoryDevicesDialog(f);
      });
    }
    // Pending section.
    if ((pending||[]).length === 0) {
      view.querySelector("#pending-list").innerHTML = `<p class="muted">暂无待审核用户</p>`;
    } else {
      view.querySelector("#pending-count").textContent = `(${pending.length})`;
      const ptbl = `<table>
        <thead><tr><th>ID</th><th>用户名</th><th>申请工厂</th><th>注册时间</th><th>操作</th></tr></thead>
        <tbody>
          ${pending.map(u => `
            <tr>
              <td>${u.id}</td>
              <td><b>${escapeHtml(u.username)}</b></td>
              <td>${escapeHtml(factoryName(u.pending_factory_id))}</td>
              <td class="muted">${(u.created_at||"").slice(0,19).replace("T"," ")}</td>
              <td class="actions">
                <button class="ghost" data-approve="${u.id}" data-name="${escapeHtml(u.username)}">通过</button>
                <button class="danger" data-reject="${u.id}" data-name="${escapeHtml(u.username)}">拒绝</button>
              </td>
            </tr>`).join("")}
        </tbody></table>`;
      view.querySelector("#pending-list").innerHTML = ptbl;
      view.querySelectorAll("[data-approve]").forEach(b => b.onclick = async () => {
        if (!confirm(`通过用户 ${b.dataset.name} 的注册申请？`)) return;
        try { await api(`/api/users/${b.dataset.approve}/approve`, { method: "PUT" }); toast("已审核通过"); renderUsers(view); }
        catch (e) { toast(e.message, true); }
      });
      view.querySelectorAll("[data-reject]").forEach(b => b.onclick = async () => {
        if (!confirm(`拒绝用户 ${b.dataset.name} 的注册申请并删除该账号？`)) return;
        try { await api(`/api/users/${b.dataset.reject}/reject`, { method: "PUT" }); toast("已拒绝"); renderUsers(view); }
        catch (e) { toast(e.message, true); }
      });
    }
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

function renderFactoryMulti(selected) {
  const facs = state.factories || [];
  if (!facs.length) return `<p class="muted" style="margin:4px 0">暂无工厂，请先在"工厂管理"中创建。</p>`;
  return facs.map(f =>
    `<label class="chip"><input type="checkbox" value="${f.id}" ${selected.includes(f.id) ? "checked" : ""}/><span>${escapeHtml(f.name)}</span></label>`
  ).join("");
}

// Admin creates a regular user in their own factory.
function editEmployeeDialog() {
  const view = document.getElementById("view");
  const isSuper = state.me.role === "super_admin";
  const myFactories = state.factories || [];
  // For non-super admin, factory is fixed to their only factory.
  const fixedFactory = !isSuper ? myFactories[0] : null;
  view.innerHTML = `
    <div class="card" style="max-width:480px;margin:0 auto">
      <h2>\u65b0\u589e\u5458\u5de5</h2>
      <label>\u7528\u6237\u540d (3-32)</label><input id="u"/>
      <label>\u5bc6\u7801 (\u22656)</label><input id="p" type="password"/>
      ${isSuper ? `
      <label>\u6240\u5c5e\u5de5\u5382</label>
      <select id="fac">
        ${myFactories.map(f => `<option value="${f.id}">${escapeHtml(f.name)}</option>`).join("")}
      </select>` : `
      <label>\u6240\u5c5e\u5de5\u5382</label>
      <p style="margin:4px 0 8px">${escapeHtml(fixedFactory ? fixedFactory.name : "")}</p>`}
      <div style="margin-top:14px" class="actions">
        <button id="ok">\u521b\u5efa</button>
        <button class="ghost" id="cancel">\u53d6\u6d88</button>
      </div>
    </div>`;
  view.querySelector("#cancel").onclick = () => renderUsers(view);
  view.querySelector("#ok").onclick = async () => {
    const fid = isSuper
      ? parseInt(view.querySelector("#fac").value, 10)
      : (fixedFactory ? fixedFactory.id : 0);
    if (!fid) { toast("\u6ca1\u6709\u53ef\u7528\u5de5\u5382", true); return; }
    try {
      await api("/api/users", {
        method: "POST",
        body: { username: view.querySelector("#u").value, password: view.querySelector("#p").value, role: "user", factory_ids: [fid] },
      });
      toast("\u5458\u5de5\u5df2\u521b\u5efa");
      await renderUsers(view);
    } catch (e) { toast(e.message, true); }
  };
}

// Admin sets which devices are visible to all users of a factory.
async function editFactoryDevicesDialog(factory) {
  const view = document.getElementById("view");
  view.innerHTML = `<div class="card" style="max-width:600px;margin:0 auto">
    <h2>${escapeHtml(factory.name)} — 可见设备</h2>
    <p class="muted">勾选员工可见的设备；不勾选任何设备则默认全部可见。</p>
    <div id="dev-list">加载中...</div>
    <div style="margin-top:14px" class="actions">
      <button id="ok">保存</button>
      <button class="ghost" id="cancel">取消</button>
    </div>
  </div>`;
  view.querySelector("#cancel").onclick = () => renderUsers(view);
  try {
    const [allDevs, curJ] = await Promise.all([
      api("/api/devices"),
      api(`/api/factories/${factory.id}/devices`),
    ]);
    const cur = new Set((curJ.device_ids || []));
    const facDevs = (allDevs || []).filter(d => d.factory_id === factory.id);
    if (facDevs.length === 0) {
      view.querySelector("#dev-list").innerHTML = `<p class="muted">该工厂暂无设备</p>`;
    } else {
      view.querySelector("#dev-list").innerHTML = facDevs.map(d =>
        `<label class="chip"><input type="checkbox" value="${d.id}" ${cur.has(d.id) ? "checked" : ""}/>` +
        `<span>${escapeHtml(d.name)} <span class="muted">${d.vid}-${d.pid}</span></span></label>`
      ).join("");
    }
    view.querySelector("#ok").onclick = async () => {
      const ids = [...view.querySelectorAll("#dev-list input:checked")].map(c => parseInt(c.value, 10));
      try {
        await api(`/api/factories/${factory.id}/devices`, { method: "PUT", body: { device_ids: ids } });
        toast("已保存");
        await renderUsers(view);
      } catch (e) { toast(e.message, true); }
    };
  } catch (e) {
    view.querySelector("#dev-list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

function editUserDialog(existing) {
  const isNew = !existing;
  const view = document.getElementById("view");
  const selected = existing ? (existing.factory_ids || []) : [];
  view.innerHTML = `
    <div class="card" style="max-width:560px;margin:0 auto">
      <h2>${isNew ? "新增用户" : "编辑用户 #" + existing.id}</h2>
      <label>用户名</label>
      <input id="u" value="${existing ? escapeHtml(existing.username) : ""}" ${existing ? "readonly" : ""}/>
      <label>角色</label>
      <select id="r">
        <option value="user">普通用户</option>
        <option value="admin">管理员</option>
        <option value="super_admin">超级管理员</option>
      </select>
      <label>所属工厂 <span class="muted" id="fac-hint">(普通用户选 1 个，管理员可多选，超级管理员无需)</span></label>
      <div id="facs" class="chip-group">${renderFactoryMulti(selected)}</div>
      <label>${isNew ? "密码 (≥6)" : "新密码 (留空则不改)"}</label>
      <input id="p" type="password"/>
      <div style="margin-top:14px" class="actions">
        <button id="ok">${isNew ? "创建" : "保存"}</button>
        <button class="ghost" id="cancel">取消</button>
      </div>
    </div>`;
  if (existing) view.querySelector("#r").value = existing.role;
  const facsBox = view.querySelector("#facs");
  const roleSel = view.querySelector("#r");
  const applyRole = () => {
    const role = roleSel.value;
    const single = role === "user";
    const disabled = role === "super_admin";
    facsBox.classList.toggle("single", single);
    facsBox.classList.toggle("disabled", disabled);
    facsBox.querySelectorAll("input").forEach(i => { i.disabled = disabled; });
    if (disabled) facsBox.querySelectorAll("input:checked").forEach(i => i.checked = false);
  };
  applyRole();
  roleSel.onchange = applyRole;
  facsBox.addEventListener("change", (e) => {
    if (roleSel.value === "user" && e.target.checked) {
      facsBox.querySelectorAll("input").forEach(i => { if (i !== e.target) i.checked = false; });
    }
  });
  view.querySelector("#cancel").onclick = () => { renderUsers(view); };
  view.querySelector("#ok").onclick = async () => {
    const role = view.querySelector("#r").value;
    const facs = [...view.querySelectorAll("#facs input:checked")].map(c => parseInt(c.value, 10));
    if (role !== "super_admin") {
      if (role === "user" && facs.length !== 1) { toast("普通用户必须选中恰好 1 个工厂", true); return; }
      if (role === "admin" && facs.length < 1) { toast("管理员必须选中至少 1 个工厂", true); return; }
    }
    try {
      if (isNew) {
        await api("/api/users", {
          method: "POST",
          body: {
            username: view.querySelector("#u").value,
            password: view.querySelector("#p").value,
            role,
            factory_ids: role === "super_admin" ? [] : facs,
          },
        });
      } else {
        const body = { role };
        const pw = view.querySelector("#p").value;
        if (pw) body.password = pw;
        await api("/api/users/" + existing.id, { method: "PUT", body });
        if (role !== "super_admin") {
          await api("/api/users/" + existing.id + "/factories",
            { method: "PUT", body: { factory_ids: facs } });
        }
      }
      await renderUsers(view);
      toast("已保存");
    } catch (e) { toast(e.message, true); }
  };
}

// Admin/super: set which devices a regular user can see.
async function editUserDevicesDialog(user) {
  const view = document.getElementById("view");
  view.innerHTML = `<div class="card"><h2>可见设备 — ${escapeHtml(user.username)}</h2>
    <p class="muted">未勾选任何项时，该用户可看到其工厂下的全部设备。</p>
    <div id="list" class="chk-list">加载中…</div>
    <div style="margin-top:14px" class="actions">
      <button id="ok">保存</button>
      <button class="ghost" id="cancel">取消</button>
    </div>
  </div>`;
  view.querySelector("#cancel").onclick = () => renderUsers(view);
  try {
    const [devs, wl] = await Promise.all([
      api("/api/devices"),
      api("/api/users/" + user.id + "/devices"),
    ]);
    const wset = new Set(wl.device_ids || []);
    // Only show devices in the user's factories.
    const ufacs = new Set(user.factory_ids || []);
    const visible = (devs||[]).filter(d => ufacs.has(d.factory_id));
    view.querySelector("#list").innerHTML = visible.length ? visible.map(d =>
      `<label class="chk"><input type="checkbox" value="${d.id}" ${wset.has(d.id)?"checked":""}/>
         <code>${escapeHtml(d.vid)}-${escapeHtml(d.pid)}</code> ${escapeHtml(d.name)}
         <span class="muted">(${escapeHtml(factoryName(d.factory_id))})</span></label>`
    ).join("") : "<p class='muted'>该用户工厂下暂无设备</p>";
    view.querySelector("#ok").onclick = async () => {
      const ids = [...view.querySelectorAll("#list input:checked")].map(c => parseInt(c.value, 10));
      try {
        await api("/api/users/" + user.id + "/devices",
          { method: "PUT", body: { device_ids: ids } });
        await renderUsers(view);
        toast("已保存");
      } catch (e) { toast(e.message, true); }
    };
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

// ---------- factories (super_admin only) ----------
async function renderFactories(view) {
  view.innerHTML = `<div class="card"><h2>工厂管理</h2>
    <div id="list">加载中…</div>
    <div style="margin-top:14px"><button id="add">+ 新增工厂</button></div>
  </div>`;
  view.querySelector("#add").onclick = async () => {
    const name = prompt("工厂名称");
    if (!name) return;
    try {
      await api("/api/factories", { method: "POST", body: { name } });
      await loadFactories(true);
      renderFactories(view);
      toast("已创建");
    } catch (e) { toast(e.message, true); }
  };
  try {
    const list = await loadFactories(true);
    view.querySelector("#list").innerHTML = list.length ? `
      <table><thead><tr><th>ID</th><th>名称</th><th>创建时间</th><th>操作</th></tr></thead>
      <tbody>${list.map(f => `
        <tr><td>${f.id}</td><td><b>${escapeHtml(f.name)}</b></td>
        <td class="muted">${(f.created_at||"").slice(0,19).replace("T"," ")}</td>
        <td class="actions">
          <button class="ghost" data-rename="${f.id}" data-name="${escapeHtml(f.name)}">重命名</button>
          <button class="danger" data-del="${f.id}" data-name="${escapeHtml(f.name)}">删除</button>
        </td></tr>`).join("")}</tbody></table>` : "<p class='muted'>暂无工厂</p>";
    view.querySelectorAll("[data-rename]").forEach(b => b.onclick = async () => {
      const name = prompt("新名称", b.dataset.name);
      if (!name || name === b.dataset.name) return;
      try {
        await api("/api/factories/" + b.dataset.rename, { method: "PUT", body: { name } });
        await loadFactories(true); renderFactories(view); toast("已保存");
      } catch (e) { toast(e.message, true); }
    });
    view.querySelectorAll("[data-del]").forEach(b => b.onclick = async () => {
      if (!confirm(`删除工厂 ${b.dataset.name}？该工厂下的设备会变成未分配。`)) return;
      try {
        await api("/api/factories/" + b.dataset.del, { method: "DELETE" });
        await loadFactories(true); renderFactories(view); toast("已删除");
      } catch (e) { toast(e.message, true); }
    });
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

// Admin/super: set which devices a regular user can see.
async function editUserDevicesDialog(user) {
  const view = document.getElementById("view");
  view.innerHTML = `<div class="card"><h2>可见设备 — ${escapeHtml(user.username)}</h2>
    <p class="muted">未勾选任何项时，该用户可看到其工厂下的全部设备。</p>
    <div id="list" class="chk-list">加载中…</div>
    <div style="margin-top:14px" class="actions">
      <button id="ok">保存</button>
      <button class="ghost" id="cancel">取消</button>
    </div>
  </div>`;
  view.querySelector("#cancel").onclick = () => renderUsers(view);
  try {
    const [devs, wl] = await Promise.all([
      api("/api/devices"),
      api("/api/users/" + user.id + "/devices"),
    ]);
    const wset = new Set(wl.device_ids || []);
    // Only show devices in the user's factories.
    const ufacs = new Set(user.factory_ids || []);
    const visible = (devs||[]).filter(d => ufacs.has(d.factory_id));
    view.querySelector("#list").innerHTML = visible.length ? visible.map(d =>
      `<label class="chk"><input type="checkbox" value="${d.id}" ${wset.has(d.id)?"checked":""}/>
         <code>${escapeHtml(d.vid)}-${escapeHtml(d.pid)}</code> ${escapeHtml(d.name)}
         <span class="muted">(${escapeHtml(factoryName(d.factory_id))})</span></label>`
    ).join("") : "<p class='muted'>该用户工厂下暂无设备</p>";
    view.querySelector("#ok").onclick = async () => {
      const ids = [...view.querySelectorAll("#list input:checked")].map(c => parseInt(c.value, 10));
      try {
        await api("/api/users/" + user.id + "/devices",
          { method: "PUT", body: { device_ids: ids } });
        await renderUsers(view);
        toast("已保存");
      } catch (e) { toast(e.message, true); }
    };
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

// ---------- factories (super_admin only) ----------
async function renderFactories(view) {
  view.innerHTML = `<div class="card"><h2>工厂管理</h2>
    <div id="list">加载中…</div>
    <div style="margin-top:14px"><button id="add">+ 新增工厂</button></div>
  </div>`;
  view.querySelector("#add").onclick = async () => {
    const name = prompt("工厂名称");
    if (!name) return;
    try {
      await api("/api/factories", { method: "POST", body: { name } });
      await loadFactories(true);
      renderFactories(view);
      toast("已创建");
    } catch (e) { toast(e.message, true); }
  };
  try {
    const list = await loadFactories(true);
    view.querySelector("#list").innerHTML = list.length ? `
      <table><thead><tr><th>ID</th><th>名称</th><th>创建时间</th><th>操作</th></tr></thead>
      <tbody>${list.map(f => `
        <tr><td>${f.id}</td><td><b>${escapeHtml(f.name)}</b></td>
        <td class="muted">${(f.created_at||"").slice(0,19).replace("T"," ")}</td>
        <td class="actions">
          <button class="ghost" data-rename="${f.id}" data-name="${escapeHtml(f.name)}">重命名</button>
          <button class="danger" data-del="${f.id}" data-name="${escapeHtml(f.name)}">删除</button>
        </td></tr>`).join("")}</tbody></table>` : "<p class='muted'>暂无工厂</p>";
    view.querySelectorAll("[data-rename]").forEach(b => b.onclick = async () => {
      const name = prompt("新名称", b.dataset.name);
      if (!name || name === b.dataset.name) return;
      try {
        await api("/api/factories/" + b.dataset.rename, { method: "PUT", body: { name } });
        await loadFactories(true); renderFactories(view); toast("已保存");
      } catch (e) { toast(e.message, true); }
    });
    view.querySelectorAll("[data-del]").forEach(b => b.onclick = async () => {
      if (!confirm(`删除工厂 ${b.dataset.name}？该工厂下的设备会变成未分配。`)) return;
      try {
        await api("/api/factories/" + b.dataset.del, { method: "DELETE" });
        await loadFactories(true); renderFactories(view); toast("已删除");
      } catch (e) { toast(e.message, true); }
    });
  } catch (e) {
    view.querySelector("#list").innerHTML = `<p class="muted">加载失败: ${escapeHtml(e.message)}</p>`;
  }
}

// ---------- boot ----------
render();
