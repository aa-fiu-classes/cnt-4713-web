---
layout: page
title: Weekly (tentative) Schedule
group: Weekly Schedule
---

<table class="table table-striped table-hover table-bordered">
  <tr>
    <th>Week</th>
    <th>Dates</th>
    <th>Week's Module</th>
    <th>Exams Due</th>
    <th>Assignment Due</th>
    <th>Project Due</th>
    <th>Lecture Questions Due</th>
  </tr>
{% for item in site.data.spreadsheets.schedule %}
  <tr>
    <td>{{ item.week }}</td>
    <td>{{ item.from }} - {{ item.to }}</td>
    <td>{{ item.module }}</td>
    <td>{{ item.exams }}</td>
    <td>{{ item.homeworks }}</td>
    <td>{{ item.projects }}</td>
    <td>{{ item.lectures }}</td>
  </tr>
{% endfor %}
</table>
